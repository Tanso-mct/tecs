#include "pch.h"
#include "tecs/system.h"

namespace tecs
{

TaskInfo::TaskInfo(std::string name, uint32_t tag)
    : name_(std::move(name)), tag_(tag)
{
}

std::string_view TaskInfo::GetName() const
{
    return name_;
}

uint32_t TaskInfo::GetTag() const
{
    return tag_;
}

TaskResult::TaskResult(bool success, std::string message)
    : success_(success), message_(std::move(message))
{
}

bool TaskResult::operator()() const
{
    return success_;
}

std::string_view TaskResult::GetMessage() const
{
    return message_;
}

Task::Task(TaskInfo info)
    : info_(std::move(info))
{
}

TaskInfo Task::GetInfo() const
{
    return info_;
}

TaskCallerInfo::TaskCallerInfo(std::string file, int line, std::string function)
    : file_(std::move(file)), line_(line), function_(std::move(function))
{
}

std::string_view TaskCallerInfo::GetFile() const
{
    return file_;
}

int TaskCallerInfo::GetLine() const
{
    return line_;
}

std::string_view TaskCallerInfo::GetFunction() const
{
    return function_;
}

TaskSet::TaskSet(std::unique_ptr<Task> task, TaskCallerInfo caller_info)
    : task(std::move(task)), caller_info(std::move(caller_info))
{
}

void TaskList::AddTask(std::unique_ptr<Task> task, TaskCallerInfo caller_info)
{
    TaskSet task_set(std::move(task), std::move(caller_info));
    tasks_.push_back(std::move(task_set));
}

std::vector<TaskSet>& TaskList::GetTasks()
{
    return tasks_;
}

void TaskListQueue::Enqueue(TaskList&& tasks)
{
    queue_.push(std::move(tasks));
}

TaskList TaskListQueue::Dequeue()
{
    if (queue_.empty())
        return TaskList();

    TaskList tasks = std::move(queue_.front());
    queue_.pop();
    return tasks;
}

bool TaskProcessor::FlushTasks(TaskListQueue& task_queue, Port& write_port, Port& read_port)
{
    while (true)
    {
        // Dequeue the next task list from the queue
        TaskList tasks = task_queue.Dequeue();
        if (tasks.GetTasks().empty())
            break;

        for (TaskSet& task_set : tasks.GetTasks())
        {
            // Get the task from the task set
            std::unique_ptr<Task>& task = task_set.task;
            if (!task)
                continue;

            {
                // Update the current task information before executing the task
                std::lock_guard<std::mutex> lock(mutex_);
                current_task_info_ = task->GetInfo();
            }

            TaskResult success = task->Execute(write_port, read_port);
            if (!success())
                return false;

            {
                // Update the current task information after executing the task
                std::lock_guard<std::mutex> lock(mutex_);
                current_task_info_ = TaskInfo();
            }
        }
    }

    return true;
}

TaskInfo TaskProcessor::GetCurrentTaskInfo() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return current_task_info_;
}

System::System()
    : task_processor_(), task_queue_(), write_port_(nullptr), read_port_(nullptr)
{
}

void System::SubmitTaskList(TaskList&& task_list)
{
    task_queue_.Enqueue(std::move(task_list));
}

const Port& System::GetReadPort() const
{
    assert(read_port_ != nullptr && "Read port is not initialized.");
    return *read_port_;
}

bool System::FlushTasks()
{
    assert(write_port_ != nullptr && "Write port is not initialized.");
    assert(read_port_ != nullptr && "Read port is not initialized.");
    return task_processor_.FlushTasks(task_queue_, *write_port_, *read_port_);
}

TaskInfo System::GetCurrentTaskInfo() const
{
    return task_processor_.GetCurrentTaskInfo();
}

bool System::Initialize(std::unique_ptr<Port> write_port, std::unique_ptr<Port> read_port)
{
    assert(write_port != nullptr && "Write port cannot be null.");
    assert(read_port != nullptr && "Read port cannot be null.");

    write_port_ = std::move(write_port);
    read_port_ = std::move(read_port);
    return true;
}

uint32_t System::next_id_ = 0;

SystemView::SystemView(System& system)
    : system_(system)
{
}

void SystemView::SubmitTaskList(TaskList&& task_list)
{
    system_.SubmitTaskList(std::move(task_list));
}

const Port& SystemView::GetReadPort() const
{
    return system_.GetReadPort();
}

std::unique_ptr<SystemView> SystemView::Clone() const
{
    return std::make_unique<SystemView>(system_);
}

void SystemViewRegistry::RegisterView(uint32_t system_id, std::unique_ptr<SystemView> view)
{
    assert(view != nullptr && "System view cannot be null.");
    views_[system_id] = std::move(view);
}

std::unique_ptr<SystemView> SystemViewRegistry::GetView(uint32_t system_id) const
{
    auto it = views_.find(system_id);
    if (it != views_.end())
        return it->second->Clone(); // Return a clone of the system view to ensure unique ownership

    return nullptr; // Return nullptr if the system view is not found
}

} // namespace tecs