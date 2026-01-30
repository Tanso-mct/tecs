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

Service::Service(JobScheduler& job_scheduler, std::unique_ptr<Context> context, std::unique_ptr<Query> query) :
    job_scheduler_(job_scheduler), 
    context_(std::move(context)),
    query_(std::move(query))
{
    assert(query_ && "Service Query cannot be nullptr");
}

void Service::SumbitTaskList(TaskList tasks)
{
    // Enqueue the provided task list
    task_list_queue_.Enqueue(std::move(tasks));
}

const Service::Query& Service::GetQuery() const
{
    assert(query_ && "Service Query is nullptr");
    return *query_;
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

ServiceProxy::ServiceProxy(Service& service) : 
    service_(service)
{
}

void ServiceProxy::SumbitTaskList(Service::TaskList tasks)
{
    service_.SumbitTaskList(std::move(tasks));
}

const Service::Query& ServiceProxy::GetQuery() const
{
    return service_.GetQuery();
}

} // namespace tecs