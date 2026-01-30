#pragma once

// STL
#include <functional>
#include <condition_variable>
#include <atomic>
#include <mutex>
#include <memory>
#include <queue>
#include <thread>

namespace tecs
{

class JobHandle
{
public:
    class JobState
    {
    public:
        JobState() = default;
        ~JobState() = default;

        // Allow JobHandle to access private members
        friend class JobHandle;

        /**
         * @brief
         * Mark the job as completed and notify all waiting threads.
         */
        void MarkCompleted();

    private:
        // Atomic flag to indicate job completion
        std::atomic<bool> completed_{false};

        // Condition variable to notify waiting threads
        std::condition_variable cv_ = {};

        // Mutex for synchronizing access to the condition variable
        std::mutex mutex_ = {};
    };

    /**
     * @brief Construct a new Job Handle object
     * 
     * @param state
     * Shared pointer to the JobState
     */
    JobHandle(std::shared_ptr<JobState> state);

    ~JobHandle() = default;

    /**
     * @brief
     * Wait for the job to complete.
     * This function blocks until the job is marked as completed.
     */
    void Wait();

private:
    // Shared pointer to the JobState
    std::shared_ptr<JobState> state_ = nullptr;
};

/**
 * @brief 
 * Job Scheduler class responsible for managing job execution.
 * Currently a placeholder with no implemented functionality.
 */
class JobScheduler
{
public:
    JobScheduler() = default;
    ~JobScheduler() = default;

    /**
     * @brief
     * Alias for a job function
     */
    using Job = std::function<void()>;

    /**
     * @brief
     * Schedule a job for execution.
     * 
     * @param job
     * The job to be scheduled.
     * 
     * @return JobHandle
     * Handle to the scheduled job.
     * You can use this handle to wait for job completion.
     */
    JobHandle ScheduleJob(Job&& job);

private:
    /**
     * @brief
     * Struct representing a scheduled job along with its state.
     */
    struct JobSet
    {
        Job job;
        std::shared_ptr<JobHandle::JobState> state;
    };

    // Queue to hold scheduled jobs
    std::queue<JobSet> job_queue_;

    // Mutex for thread-safe access to the job queue
    std::mutex mutex_;

    // Condition variable to notify worker thread of new jobs
    std::condition_variable cv_;

    // Atomic flag to signal the worker thread to stop
    std::atomic<bool> stop_flag_{false};

    // Worker thread for processing jobs
    std::thread worker_thread_;

    /**
     * @brief
     * Worker function that processes jobs from the queue.
     */
    void Work();
};

} // namespace tecs