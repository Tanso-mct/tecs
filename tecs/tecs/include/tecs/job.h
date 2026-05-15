#pragma once

// STL
#include <string>
#include <string_view>
#include <memory>

namespace tecs
{

class JobType
{
public:
    /**
     * @brief : Construct a new JobType object with the specified parameters
     * @param name : The name of the job type
     * @param id : A unique identifier for the job type
     * @param priority : The priority level of the job type
     * @param cost : An estimate of the computational cost of jobs of this type
     */
    JobType(std::string name, uint32_t id, uint32_t priority, uint32_t cost);

    ~JobType() = default;

    /**
     * @brief : Get the name of the job type
     * @return std::string_view : The name of the job type
     */
    std::string_view GetName() const { return name_; }

    /**
     * @brief : Get the unique identifier of the job type
     * @return uint32_t : The unique identifier of the job type
     */
    uint32_t GetID() const { return id_; }

    /**
     * @brief : Get the priority level of the job type
     * @return uint32_t : The priority level of the job type
     */
    uint32_t GetPriority() const { return priority_; }

    /**
     * @brief : Get the estimated computational cost of jobs of this type
     * @return uint32_t : The estimated computational cost of jobs of this type
     */
    uint32_t GetCost() const { return cost_; }

private:
    // The name of the job type, used for identification and categorization
    std::string name_;

    // A unique identifier for the job type, used for efficient lookup and management
    uint32_t id_;

    // The priority level of the job type, which can be used to determine the order of job execution
    uint32_t priority_;

    // An estimate of the computational cost
    uint32_t cost_;
};

class JobState
{
public:
    /**
     * @brief : Construct a new JobState object with the specified job type and completion status
     * @param is_completed : A weak pointer to a boolean that indicates whether the job has been completed
     */
    JobState(std::weak_ptr<bool> is_completed);

    ~JobState() = default;

    /**
     * @brief : Check if the job has been completed
     * @return bool : True if the job is completed, false otherwise
     */
    bool IsCompleted() const;

    /**
     * @brief : Wait for the job to be completed and return the completion status
     * @return bool : True if the job is completed, false otherwise
     */
    bool WaitForCompletion() const;

private:
    // A weak pointer to a boolean that indicates whether the job has been completed
    std::weak_ptr<bool> is_completed_;
};

// class WorkerThread
// {
// public:
//     WorkerThread() = default;
//     ~WorkerThread() = default;

//     /**
//      * @brief
//      * Start the worker thread to execute the given function
//      * 
//      * @param func
//      * The function to be executed in the worker thread
//      */
//     void Start(std::function<void()> func);

//     /**
//      * @brief
//      * Stop the worker thread and wait for it to finish execution
//      */
//     void Stop();

//     /**
//      * @brief
//      * Check if the worker thread is currently running (executing a job)
//      * 
//      * @return true
//      * If the worker thread is currently executing a job
//      * 
//      * @return false
//      * If the worker thread is idle (not executing a job)
//      */
//     bool IsRunning() const;

//     /**
//      * @brief
//      * Dispatch a job to the worker thread for execution
//      * 
//      * @param job
//      * The job to be executed by the worker thread
//      * 
//      * @return bool 
//      */
//     bool DispatchJob(std::unique_ptr<Job> job);


// };


// class ThreadProvider
// {
// public:
//     ThreadProvider() = default;
//     ~ThreadProvider() = default;

//     /**
//      * @brief
//      * Get the number of CPU cores available
//      * 
//      * @return uint32_t
//      * The number of CPU cores
//      */
//     uint32_t GetCpuCoreCount();

//     /**
//      * @brief
//      * Create a new thread to execute the given function
//      * 
//      * @param func
//      * The function to be executed in the new thread
//      */
//     void CreateThread(std::function<void()> func);

//     /**
//      * @brief
//      * Create multiple threads to execute the given function
//      * 
//      * @param func
//      * The function to be executed in each new thread
//      * 
//      * @param num_threads
//      * The number of threads to create
//      */
//     void CreateThreads(std::function<void()> func, uint32_t num_threads);



// private:
//     // Vector to hold created threads
//     std::vector<std::thread> threads_;
// };

// // Forward declaration
// class JobHandle;

// /**
//  * @brief
//  * Class representing the state of a job
//  * This class is used internally by JobHandle to manage job completion
//  */
// class JobState
// {
// public:
//     JobState() = default;
//     ~JobState() = default;

//     // Allow JobHandle to access private members
//     friend class JobHandle;

//     /**
//      * @brief
//      * Mark the job as completed and notify all waiting threads
//      */
//     void MarkCompleted();

// private:
//     // Atomic flag to indicate job completion
//     std::atomic<bool> completed_{false};

//     // Condition variable to notify waiting threads
//     std::condition_variable cv_ = {};

//     // Mutex for synchronizing access to the condition variable
//     std::mutex mutex_ = {};
// };

// /**
//  * @brief
//  * Handle for a scheduled job
//  * Allows waiting for job completion
//  */
// class JobHandle
// {
// public:
//     /**
//      * @brief
//      * Construct a new Job Handle object
//      * 
//      * @param state
//      * Shared pointer to the JobState
//      */
//     JobHandle(std::shared_ptr<JobState> state);

//     ~JobHandle() = default;

//     /**
//      * @brief
//      * Wait for the job to complete
//      * This function blocks until the job is marked as completed
//      */
//     void Wait();

// private:
//     // Shared pointer to the JobState
//     std::shared_ptr<JobState> state_ = nullptr;
// };

// /**
//  * @brief
//  * Job class representing a unit of work to be executed
//  * This class encapsulates a function to be run as a job
//  */
// class Job
// {
// public:
//     /**
//      * @brief
//      * Alias for the job function
//      */
//     using JobFunc = std::function<void()>;

//     /**
//      * @brief
//      * Construct a new Job object
//      * 
//      * @param func 
//      * The function to be executed as part of the job
//      * If nullptr is passed, will be asserted
//      */
//     Job(JobFunc func);
    
//     ~Job() = default;

//     /**
//      * @brief
//      * Execute the job function
//      */
//     void Execute();

// private:
//     // The function to be executed as part of the job
//     JobFunc func_ = nullptr;
// };

// /**
//  * @brief
//  * Struct representing a scheduled job along with its state
//  */
// class JobSet
// {
// public:
//     /**
//      * @brief
//      * Construct a new Job Set object
//      * 
//      * @param job 
//      * The job to be scheduled
//      * 
//      * @param state 
//      * Shared pointer to the JobState
//      */
//     JobSet(Job job, std::shared_ptr<JobState> state);

//     Job job_;
//     std::shared_ptr<JobState> state_;
// };

// /**
//  * @brief 
//  * Job Scheduler class responsible for managing job execution
//  * Currently a placeholder with no implemented functionality
//  */
// class JobScheduler
// {
// public:
//     /**
//      * @brief 
//      * Construct a new Job Scheduler object
//      * Worker thread is started upon construction
//      * 
//      * @param num_worker_threads
//      * Number of worker threads to process jobs
//      */
//     JobScheduler(uint32_t num_worker_threads);

//     /**
//      * @brief
//      * Destroy the Job Scheduler object
//      * Worker thread is stopped and joined upon destruction
//      */
//     ~JobScheduler();

//     /**
//      * @brief
//      * Schedule a job for execution
//      * 
//      * @param job
//      * The job to be scheduled
//      * 
//      * @return JobHandle
//      * Handle to the scheduled job
//      * You can use this handle to wait for job completion
//      */
//     JobHandle ScheduleJob(Job job);

// private:
//     // Queue to hold scheduled jobs
//     std::queue<std::unique_ptr<JobSet>> job_queue_;

//     // Mutex for thread-safe access to the job queue
//     std::mutex mutex_;

//     // Condition variable to notify worker thread of new jobs
//     std::condition_variable cv_;

//     // Atomic flag to signal the worker thread to stop
//     std::atomic<bool> stop_flag_{false};

//     // Worker threads for processing jobs
//     std::vector<std::thread> worker_threads_;

//     /**
//      * @brief
//      * Worker function that processes jobs from the queue
//      */
//     void Work();
// };

} // namespace tecs