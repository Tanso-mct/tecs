#pragma once

// STL
#include <string>
#include <string_view>
#include <memory>
#include <future>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <vector>
#include <queue>

namespace tecs
{

constexpr const char* DEFAULT_JOB_TYPE_NAME = "Default";
const uint32_t DEFAULT_JOB_TYPE_ID = 0;

class JobType
{
public:
    /**
     * @brief : Construct a new JobType object with the specified parameters
     * @param name : The name of the job type
     * @param id : A unique identifier for the job type
     */
    JobType(std::string name, uint32_t id);

    /**
     * @brief Construct a new Job Type object
     */
    JobType();

    ~JobType() = default;

    /**
     * @brief Copy constructor for JobType
     * @param other The other JobType object to copy from
     */
    JobType(const JobType& other);

    /**
     * @brief : Equality operator to compare two JobType objects
     * @param other : The other JobType object to compare with
     * @return bool : True if both JobType objects are equal, false otherwise
     */
    bool operator==(const JobType& other) const;

    /**
     * @brief : Non-equality operator to compare two JobType objects
     * @param other : The other JobType object to compare with
     * @return bool : True if both JobType objects are not equal, false otherwise
     */
    bool operator!=(const JobType& other) const;

    /**
     * @brief : Get the name of the job type
     * @return std::string_view : The name of the job type
     */
    std::string_view GetName() const;

    /**
     * @brief : Get the unique identifier of the job type
     * @return uint32_t : The unique identifier of the job type
     */
    uint32_t GetID() const;

    /**
     * @brief : Update the name and ID of the job type
     * @param name : The new name of the job type
     * @param id : The new unique identifier for the job type
     */
    void Update(std::string name, uint32_t id);

    /**
     * @brief Reset the job type to default state
     */
    void Reset();

private:
    // The name of the job type, used for identification and categorization
    std::string name_;

    // A unique identifier for the job type, used for efficient lookup and management
    uint32_t id_;

    // A mutex to protect access to the job type's data, allowing for thread-safe updates
    mutable std::mutex mutex_;
};

class JobState
{
public:
    /**
     * @brief : Construct a new JobState object with the specified completion future
     * @param completion : A shared future that allows multiple threads to wait for the job's completion
     */
    JobState(std::shared_future<void> completion);

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
    // A shared future that allows multiple threads to wait for the job's completion
    std::shared_future<void> completion_;

};

class Job
{
public:
    /**
     * @brief : Construct a new Job object with the specified job type and function to execute
     * @param type : The type of the job
     * @param job_func : The function that represents the work to be done by the job
     */
    Job(JobType type, std::function<void()> job_func);

    ~Job() = default;

    /**
     * @brief : Get the current state of the job
     * @return JobState : The current state of the job
     */
    JobState GetState() const;

    /**
     * @brief : Get the type of the job
     * @return JobType : The type of the job
     */
    JobType GetType() const;

    /**
     * @brief : Execute the job's function and update the completion status
     * @note : This method should be called by the worker thread that is processing the job
     */
    void Execute();

private:
    // The function that represents the work to be done by the job
    std::function<void()> job_fun_;

    // A promise that will be fulfilled when the job is completed, allowing other threads to wait for its completion
    std::shared_ptr<std::promise<void>> promise_;

    // A shared future that allows multiple threads to wait for the job's completion
    mutable std::shared_future<void> shared_;

    // The type of the job, which can be used for categorization and scheduling
    JobType type_;

};

class ThreadController
{
public:
    /**
     * @brief : Construct a new ThreadController object that manages the worker thread and job execution
     */
    ThreadController();

    ~ThreadController() = default;

    /**
     * @brief : Check if the stop signal has been set for the worker thread
     * @return bool : True if the stop signal is set, false otherwise
     */
    bool HasStopSignal() const;

    /**
     * @brief : Signal the worker thread to stop processing jobs and exit
     */
    void SignalStop();

    /**
     * @brief : Wait for a condition to be met, using the provided predicate function
     * @param predicate : A function that returns true when the condition is met, allowing the worker thread to proceed
     */
    void Wait(std::function<bool()> predicate);

    /**
     * @brief : Notify the condition variable to wake up the thread
     */
    void Notify();

private:
    // A condition variable to signal the worker thread when a new job is dispatched
    std::condition_variable condition_;

    // A mutex to protect access to condition variable
    std::mutex mutex_;

    // An atomic flag to signal the worker thread to stop processing and exit
    std::atomic<bool> stop_signal_;

};

class WorkerThread
{
public:
    /**
     * @brief : Create a new worker thread that will process jobs from the job queue
     * @param worker_loop_func : A function that represents the main loop of the worker thread
     */
    WorkerThread(std::function<void(ThreadController&, JobType&)> worker_loop_func);

    /**
     * @brief : Signal the worker thread to stop and join it
     */
    ~WorkerThread();

    /**
     * @brief : Get the unique identifier of the worker thread
     * @return uint32_t : The unique identifier of the worker thread
     */
    uint32_t GetID() const;

    /**
     * @brief : Get the type of job that the worker thread is currently processing
     * @return JobType : The type of job being processed, or a default value if the worker thread is idle
     */
    JobType GetWorkingJobType() const;

private:
    // A static variable that keeps track of the next unique identifier to assign to a new worker thread
    static uint32_t next_id_;

    // A unique identifier for the worker thread, used for tracking and management
    uint32_t id_;

    // The actual thread object that runs the worker loop
    std::thread thread_;

    // A controller that manages the worker thread's execution and stopping behavior
    ThreadController thread_controller_;

    // Job type currently being processed by the worker thread
    JobType current_job_type_;

};

class JobQueue
{
public:
    JobQueue() = default;
    ~JobQueue() = default;

    /**
     * @brief : Add a new job to the queue for execution by worker threads
     * @param job : The job to be added to the queue
     */
    void PushJob(Job job);

    /**
     * @brief : Remove and return the next job from the queue for execution by a worker thread
     * @return Job : The next job to be executed, or a default-constructed job
     */
    Job PopJob();

    /**
     * @brief : Check if the job queue is empty
     * @return bool : True if the queue is empty, false otherwise
     */
    bool IsEmpty() const;

public:
    // A queue that holds jobs waiting to be executed by worker threads
    std::queue<Job> job_queue_;

    // A mutex to protect access to the job queue
    mutable std::mutex mutex_;
};

class JobScheduler
{
public:
    /**
     * @brief : Construct a new JobScheduler object
     */
    JobScheduler();

    ~JobScheduler();

    /**
     * @brief : Schedule a job to be executed by an idle worker thread
     * @param job : The job to be scheduled
     * @return JobState : The state of the scheduled job (e.g., pending, running, completed)
     */
    JobState ScheduleJob(Job job);

    /**
     * @brief : Get a reference to the vector of worker threads managed by the job scheduler
     * @return const std::vector<WorkerThread>& : A reference to the vector of worker threads
     */
    const std::vector<std::unique_ptr<WorkerThread>>& GetWorkerThreads() const;

private:
    // A vector that holds the worker threads responsible for executing jobs
    std::vector<std::unique_ptr<WorkerThread>> worker_threads_;

    // A queue that holds jobs waiting to be executed by worker threads
    JobQueue job_queue_;

};

class JobExecutionInfo
{
public:
    /**
     * @brief : Construct a new JobExecutionInfo object with the specified worker thread ID and job type
     * @param worker_thread_id : The ID of the worker thread that is processing the job
     * @param job_type : The type of the job being processed
     */
    JobExecutionInfo(uint32_t worker_thread_id, JobType job_type);

    ~JobExecutionInfo() = default;

    /**
     * @brief : Get the ID of the worker thread that is processing the job
     * @return uint32_t : The ID of the worker thread
     */
    uint32_t GetWorkerThreadID() const;

    /**
     * @brief : Get the type of the job being processed
     * @return JobType : The type of the job being processed
     */
    JobType GetJobType() const;

private:
    //  The worker thread ID which processes the job
    uint32_t worker_thread_id_;

    //  The type of the job being processed
    JobType job_type_;

};

class JobTracker
{
public:
    /**
     * @brief : Construct a new JobTracker object with the specified worker threads
     */
    JobTracker(const std::vector<std::unique_ptr<WorkerThread>>& worker_threads);

    ~JobTracker() = default;

    /**
     * @brief : Get a list of all currently running job execution information
     * @return std::vector<JobExecutionInfo> : A vector ofoJobExecutionInfo objects representing the currently running jobs
     */
    std::vector<JobExecutionInfo> GetRunningJobInfos() const;

private:
    // A reference to the vector of worker threads, used to track the jobs currently being processed
    const std::vector<std::unique_ptr<WorkerThread>>& worker_threads_;

};

} // namespace tecs