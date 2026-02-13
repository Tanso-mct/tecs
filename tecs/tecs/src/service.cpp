#include "pch.h"
#include "tecs/service.h"

namespace tecs
{

// Initialize static member
uint32_t Service::next_id_ = 0;

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

std::unique_ptr<ServiceProxy> ServiceProxy::Clone() const
{
    return std::make_unique<ServiceProxy>(service_);
}

void ServiceProxyManager::RegisterServiceProxy(uint32_t service_id, std::unique_ptr<ServiceProxy> proxy)
{
    // Lock the mutex for thread-safe access
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = service_proxies_.find(service_id);
    if (it != service_proxies_.end())
    {
        // If a proxy for this service ID already exists, replace it
        it->second = std::move(proxy);
    }
    else
    {
        // Otherwise, insert the new proxy
        service_proxies_.emplace(service_id, std::move(proxy));
    }
}

std::unique_ptr<ServiceProxy> ServiceProxyManager::GetServiceProxy(uint32_t service_id)
{
    // Lock the mutex for thread-safe access
    std::lock_guard<std::mutex> lock(mutex_);

    // Find the ServiceProxy for the specified service ID
    auto it = service_proxies_.find(service_id);
    assert(it != service_proxies_.end() && "ServiceProxy for the specified service ID not found");

    // Return a clone of the ServiceProxy
    assert(it->second && "ServiceProxy for the specified service ID is nullptr");
    return it->second->Clone(); // Return a clone of the ServiceProxy
}

} // namespace tecs