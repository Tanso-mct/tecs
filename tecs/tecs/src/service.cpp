#include "pch.h"
#include "tecs/service.h"

namespace tecs
{

Service::Task::Task(Func func) : 
    func_(func)
{
    assert(func_ && "Task function cannot be nullptr");

    // Create default Info if none provided
    info_ = std::make_unique<Info>();
}

Service::Task::Task(Func func, std::unique_ptr<Info> info) :
    func_(func), 
    info_(std::move(info))
{
    assert(func_ && "Task function cannot be nullptr");
    assert(info_ && "Task Info cannot be nullptr");
}

bool Service::Task::Execute(Context& context, JobScheduler& job_scheduler)
{
    return func_(context, job_scheduler);
}

const Service::Task::Info& Service::Task::GetInfo() const
{
    assert(info_ && "Task Info is nullptr");
    return *info_;
}

Service::Service(JobScheduler& job_scheduler, std::unique_ptr<Context> context) :
    job_scheduler_(job_scheduler), 
    context_(std::move(context))
{
    assert(context_ && "Service Context cannot be nullptr");
}

void Service::SumbitTaskList(TaskList tasks)
{
    // Enqueue the provided task list
    task_list_queue_.Enqueue(std::move(tasks));
}

const Service::Context& Service::GetContext() const
{
    assert(context_ && "Service Context is nullptr");
    return *context_;
}

void Service::TaskListQueue::Enqueue(TaskList tasks)
{
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.emplace(std::move(tasks));
}

bool Service::TaskListQueue::Dequeue(TaskList& tasks)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty())
        return false; // If the queue is empty, return false

    // Move the front task list to the provided reference
    tasks = std::move(queue_.front());
    queue_.pop();

    return true; // Successfully dequeued a task list
}

std::vector<Service::TaskList> Service::TaskListQueue::Dequeue()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<TaskList> all_tasks;

    while (!queue_.empty())
    {
        // Move each task list to the vector
        all_tasks.emplace_back(std::move(queue_.front()));
        queue_.pop();
    }

    return all_tasks; // Return all dequeued task lists
}

ServiceProxy::ServiceProxy(Service& service) : 
    service_(service)
{
}

void ServiceProxy::SumbitTaskList(Service::TaskList tasks)
{
    service_.SumbitTaskList(std::move(tasks));
}

} // namespace tecs