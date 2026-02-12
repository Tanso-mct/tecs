#pragma once

// STL
#include <functional>
#include <condition_variable>
#include <atomic>
#include <mutex>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

namespace tecs
{

// Forward declaration
class JobHandle;

/**
 * @brief
 * Class representing the state of a job
 * This class is used internally by JobHandle to manage job completion
 */
class JobState
{
public:
    JobState() = default;
    ~JobState() = default;

    // Allow JobHandle to access private members
    friend class JobHandle;

    /**
     * @brief
     * Mark the job as completed and notify all waiting threads
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
 * @brief
 * Handle for a scheduled job
 * Allows waiting for job completion
 */
class JobHandle
{
public:
    /**
     * @brief
     * Construct a new Job Handle object
     * 
     * @param state
     * Shared pointer to the JobState
     */
    JobHandle(std::shared_ptr<JobState> state);

    ~JobHandle() = default;

    /**
     * @brief
     * Wait for the job to complete
     * This function blocks until the job is marked as completed
     */
    void Wait();

private:
    // Shared pointer to the JobState
    std::shared_ptr<JobState> state_ = nullptr;
};

/**
 * @brief
 * Job class representing a unit of work to be executed
 * This class encapsulates a function to be run as a job
 */
class Job
{
public:
    /**
     * @brief
     * Alias for the job function
     * 
     * @return true
     * If the job was successful
     * 
     * @return false
     * If the job failed
     */
    using JobFunc = std::function<void()>;

    /**
     * @brief
     * Construct a new Job object
     * 
     * @param func 
     * The function to be executed as part of the job
     * If nullptr is passed, will be asserted
     */
    Job(JobFunc func);
    
    ~Job() = default;

    /**
     * @brief
     * Execute the job function
     */
    void Execute();

private:
    // The function to be executed as part of the job
    JobFunc func_ = nullptr;
};

/**
 * @brief 
 * Job Scheduler class responsible for managing job execution
 * Currently a placeholder with no implemented functionality
 */
class JobScheduler
{
public:
    /**
     * @brief 
     * Construct a new Job Scheduler object
     * Worker thread is started upon construction
     * 
     * @param num_worker_threads
     * Number of worker threads to process jobs
     */
    JobScheduler(uint32_t num_worker_threads);

    /**
     * @brief
     * Destroy the Job Scheduler object
     * Worker thread is stopped and joined upon destruction
     */
    ~JobScheduler();

    /**
     * @brief
     * Schedule a job for execution
     * 
     * @param job
     * The job to be scheduled
     * 
     * @return JobHandle
     * Handle to the scheduled job
     * You can use this handle to wait for job completion
     */
    JobHandle ScheduleJob(Job job);

private:
    /**
     * @brief
     * Struct representing a scheduled job along with its state
     */
    class JobSet
    {
    public:
        /**
         * @brief
         * Construct a new Job Set object
         * 
         * @param job 
         * The job to be scheduled
         * 
         * @param state 
         * Shared pointer to the JobState
         */
        JobSet(Job job, std::shared_ptr<JobState> state);

        Job job_;
        std::shared_ptr<JobState> state_;
    };

    // Queue to hold scheduled jobs
    std::queue<std::unique_ptr<JobSet>> job_queue_;

    // Mutex for thread-safe access to the job queue
    std::mutex mutex_;

    // Condition variable to notify worker thread of new jobs
    std::condition_variable cv_;

    // Atomic flag to signal the worker thread to stop
    std::atomic<bool> stop_flag_{false};

    // Worker threads for processing jobs
    std::vector<std::thread> worker_thread_;

    /**
     * @brief
     * Worker function that processes jobs from the queue
     */
    void Work();
};

} // namespace tecs