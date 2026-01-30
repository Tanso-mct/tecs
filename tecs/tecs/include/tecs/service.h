#pragma once

// STL
#include <functional>
#include <vector>
#include <memory>
#include <queue>
#include <mutex>
#include <string>

// TECS
#include "tecs/job.h" // JobScheduler

namespace tecs
{

/**
 * @brief 
 * Base class for all services
 * Services are designed to provide specific functions
 */
class Service
{
public:
    /**
     * @brief
     * Interface for the context
     * It holds internally data specific to each service
     */
    class Context
    {
    public:
        virtual ~Context() = default;
    };

    /**
     * @brief
     * Class representing a task to be executed by the service
     * A task consists of a function and optional additional information
     */
    class Task
    {
    public:
        /**
         * @brief
         * Interface for additional task information
         * You can inherit this class to provide custom information for each task
         */
        class Info
        {
        public:
            virtual ~Info() = default;
        };

        /**
         * @brief 
         * Alias for a task function that takes a JobScheduler reference
         * 
         * @return true
         * If the task was successful
         * 
         * @return false
         * If the task failed
         */
        using Func = std::function<bool(Context&, JobScheduler&)>;

        /**
         * @brief
         * Construct a new Task object
         * 
         * @param func 
         * The function to be executed as part of the task
         * If nullptr is passed, will be asserted
         */
        Task(Func func);

        /**
         * @brief 
         * Construct a new Task object
         * 
         * @param func 
         * The function to be executed as part of the task
         * If nullptr is passed, will be asserted
         * 
         * @param info 
         * Additional task information
         * If nullptr is passed, will be asserted
         */
        Task(Func func, std::unique_ptr<Info> info);

        ~Task() = default;

        /**
         * @brief 
         * Execute the task function
         * 
         * @param job_scheduler 
         * Reference to the JobScheduler
         * 
         * @return true 
         * If the task was successful
         * 
         * @return false 
         * If the task failed
         */
        bool Execute(Context& context, JobScheduler& job_scheduler);

        /**
         * @brief 
         * Get the task information
         * 
         * @return const Info& 
         * Reference to the Info
         */
        const Info& GetInfo() const;

    private:
        // The function to be executed as part of the task
        Func func_ = nullptr;

        // Information about the task
        std::unique_ptr<Info> info_ = nullptr;
    };

    /**
     * @brief
     * Alias for a list of tasks
     */
    using TaskList = std::vector<Task>;

    /**
     * @brief 
     * Interface for querying the internal state of a service
     * You can inherit this interface and implement inquiry methods specific to each service
     */
    class Query
    {
    public:
        Query(const Context& context);
        virtual ~Query() = default;

    protected:
        // Reference to the Context
        const Context& context_;
    };

    /**
     * @brief
     * Construct a new Service object
     * 
     * @param job_scheduler 
     * Reference to the JobScheduler
     * 
     * @param query 
     * Unique pointer to the Query object 
     * Can not be nullptr If nullptr is passed, will be asserted
     */
    Service(JobScheduler& job_scheduler, std::unique_ptr<Context> context, std::unique_ptr<Query> query);

    /**
     * @brief Destroy the Service object
     * This class is intended to be used as a base class, so the destructor is virtual
     */
    virtual ~Service() = default;

    /**
     * @brief 
     * Submit a list of tasks to the service for execution
     * 
     * @param tasks
     * List of tasks to be submitted
     */
    void SumbitTaskList(TaskList tasks);

    /**
     * @brief 
     * Get the Query object
     * You can use this to inquire about the internal state of the service
     * 
     * @return 
     * Reference to the Query object
     */
    const Query& GetQuery() const;

    /**
     * @brief
     * Pre-update phase of the service
     * 
     * @return true 
     * If the pre-update was successful
     * 
     * @return false 
     * If the pre-update failed
     */
    virtual bool PreUpdate() = 0;

    /**
     * @brief
     * Update phase of the service
     * 
     * @return true 
     * If the update was successful
     * 
     * @return false 
     * If the update failed
     */
    virtual bool Update() = 0;

    /**
     * @brief
     * Post-update phase of the service
     * 
     * @return true 
     * If the post-update was successful
     * 
     * @return false 
     * If the post-update failed
     */
    virtual bool PostUpdate() = 0;

protected:
    /**
     * @brief
     * Task List Queue for managing submitted tasks
     * This class is thread-safe and handles the queuing of task lists
     */
    class TaskListQueue
    {
    public:
        TaskListQueue() = default;
        ~TaskListQueue() = default;

        /**
         * @brief
         * Enqueue a list of tasks into the queue
         * 
         * @param tasks
         * List of tasks to be enqueued
         */
        void Enqueue(TaskList tasks);

        /**
         * @brief
         * Dequeue a list of tasks from the queue
         * 
         * @param tasks 
         * Reference to store the dequeued list of tasks
         * TaskList will be moved to the provided reference
         * 
         * @return true 
         * if a task list was successfully dequeued
         * 
         * @return false 
         * if the queue is empty
         */
        bool Dequeue(TaskList& tasks);

    private:
        // Internal storage for the task queue
        std::queue<TaskList> queue_;

        // Mutex for thread-safe access to the queue
        std::mutex mutex_;
    };

    // Reference to the JobScheduler
    JobScheduler& job_scheduler_;

    // Unique pointer to the Context object
    std::unique_ptr<Context> context_ = nullptr;

    // Task List Queue for managing submitted tasks
    TaskListQueue task_list_queue_;

    // Unique pointer to the Query object
    std::unique_ptr<Query> query_ = nullptr;
};

class ServiceProxy
{
public:
    /**
     * @brief
     * Construct a new Service Proxy object
     * 
     * @param service 
     * Reference to the Service to be proxied
     */
    ServiceProxy(Service& service);

    /**
     * @brief
     * Wrapper for Service::SumbitTaskList
     * 
     * @param tasks 
     * List of tasks to be submitted
     */
    void SumbitTaskList(Service::TaskList tasks);

    /**
     * @brief
     * Wrapper for Service::GetQuery
     * 
     * @return 
     * Reference to the Query object of the proxied Service
     */
    const Service::Query& GetQuery() const;

private:
    // Reference to the proxied Service
    Service& service_;
};

} // namespace tecs