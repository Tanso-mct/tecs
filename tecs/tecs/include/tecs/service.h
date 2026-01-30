#pragma once

// STL
#include <functional>
#include <vector>
#include <memory>
#include <queue>
#include <mutex>

// TECS
#include "tecs/job.h" // JobScheduler

namespace tecs
{

/**
 * @brief 
 * Base class for all services.
 * Services are designed to provide specific functions.
 */
class Service
{
public:
    /**
     * @brief 
     * Alias for a task function that takes a JobScheduler reference.
     * 
     * @return true
     * If the task was successful.
     * 
     * @return false
     * If the task failed.
     */
    using Task = std::function<bool(JobScheduler&)>;

    /**
     * @brief
     * Alias for a list of tasks.
     */
    using TaskList = std::vector<Task>;

    /**
     * @brief 
     * Interface for querying the internal state of a service.
     * You can inherit this interface and implement inquiry methods specific to each service.
     */
    class Query
    {
    public:
        virtual ~Query() = default;

        // Add inquiry methods specific to each service here
    };

    /**
     * @brief
     * Construct a new Service object
     * 
     * @param job_scheduler 
     * Reference to the JobScheduler
     * 
     * @param query 
     * Unique pointer to the Query object. 
     * Can not be nullptr. If nullptr is passed, will be asserted.
     */
    Service(JobScheduler& job_scheduler, std::unique_ptr<Query> query);

    /**
     * @brief Destroy the Service object
     * This class is intended to be used as a base class, so the destructor is virtual.
     */
    virtual ~Service() = default;

    /**
     * @brief 
     * Submit a list of tasks to the service for execution.
     * 
     * @param tasks
     * List of tasks to be submitted
     */
    void SumbitTasks(TaskList&& tasks);

    /**
     * @brief 
     * Get the Query object.
     * You can use this to inquire about the internal state of the service.
     * 
     * @return 
     * Reference to the Query object.
     */
    virtual const Query& GetQuery() const;

private:
    /**
     * @brief
     * Task List Queue for managing submitted tasks.
     * This class is thread-safe and handles the queuing of task lists.
     */
    class TaskListQueue
    {
    public:
        TaskListQueue() = default;
        ~TaskListQueue() = default;

        /**
         * @brief
         * Enqueue a list of tasks into the queue.
         * 
         * @param tasks
         * List of tasks to be enqueued.
         */
        void Enqueue(TaskList&& tasks);

        /**
         * @brief
         * Dequeue a list of tasks from the queue.
         * 
         * @param tasks 
         * Reference to store the dequeued list of tasks.
         * TaskList will be moved to the provided reference.
         * 
         * @return true 
         * if a task list was successfully dequeued.
         * 
         * @return false 
         * if the queue is empty.
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

    // Task List Queue for managing submitted tasks
    TaskListQueue task_list_queue_;

    // Unique pointer to the Query object
    std::unique_ptr<Query> query_;
};

} // namespace tecs