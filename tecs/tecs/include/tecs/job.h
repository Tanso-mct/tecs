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
     */
    JobType(std::string name, uint32_t id);

    ~JobType() = default;

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

private:
    // The name of the job type, used for identification and categorization
    std::string name_;

    // A unique identifier for the job type, used for efficient lookup and management
    uint32_t id_;

};

class JobState
{
public:
    /**
     * @brief : Construct a new JobState object with the specified job type and completion status
     * @param is_completed : A pointer to a boolean that indicates whether the job has been completed
     * @param condition : A pointer to a condition variable
     * @param mutex : A pointer to a mutex to protect access to the job's completion status and condition variable
     */
    JobState(std::atomic<bool>* is_completed, std::condition_variable* condition, std::mutex* mutex);

    JobState() = default;
    ~JobState() = default;

    /**
     * @brief : Initialize the JobState object with the specified job type and completion status
     * @param is_completed : A pointer to a boolean that indicates whether the job has been completed
     * @param condition : A pointer to a condition variable
     * @param mutex : A pointer to a mutex to protect access to the job's completion status and condition variable
    */
    void Initialize(std::atomic<bool>* is_completed, std::condition_variable* condition, std::mutex* mutex);

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
    // A pointer to a boolean that indicates whether the job has been completed
    std::atomic<bool>* is_completed_;

    // A pointer to a condition variable that can be used to wait for the job's completion status to change
    std::condition_variable* condition_;

    // A pointer to a mutex to protect access to the job's completion status and condition variable
    std::mutex* mutex_;

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
     * @brief : Get the type of the job
     * @return JobType : The type of the job
     */
    JobType GetType() const;

    /**
     * @brief : Get the state of the job
     * @return std::weak_ptr<JobState> : A weak pointer to the state of the job
     */
    std::weak_ptr<JobState> GetState() const;

    /**
     * @brief : Execute the job's function
     * @note : This method should be called by the worker thread that is processing the job
     */
    void Execute();

private:
    // The type of the job, which can be used for categorization and scheduling
    JobType type_;

    // The function that represents the work to be done by the job
    std::function<void()> job_fun_;

    // A shared pointer to the state of the job
    std::shared_ptr<JobState> state_;

};

class WorkerThread
{
public:
    /**
     * @brief : Create a new worker thread that will process jobs from the job queue
     */
    WorkerThread();

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
     * @brief : Dispatch a job to be executed by the worker thread
     * @param job : The job to be executed
     * @return std::weak_ptr<JobState> : A weak pointer to the state of the dispatched job
     */
    std::weak_ptr<JobState> DispatchJob(Job job);

    /**
     * @brief : Check if the worker thread is currently idle (not processing a job)
     * @return bool : True if the worker thread is idle, false otherwise
     */
    bool IsIdle() const;

    /**
     * @brief : Get the type of job that the worker thread is currently processing
     * @return JobType : The type of job being processed, or a default value if the worker thread is idle
     */
    JobType GetWorkingJobType() const;

private:
    /**
     * @brief : The main loop of the worker thread that continuously checks for new jobs to process
     */
    void Work();

private:
    // A unique identifier for the worker thread, used for tracking and management
    uint32_t id_;

    // The actual thread object that runs the worker loop
    std::thread thread_;

    // A mutex to protect access
    mutable std::mutex mutex_;

    // A condition variable to signal the worker thread when a new job is dispatched
    std::condition_variable condition_;

    // A flag to signal the worker thread to stop processing and exit
    std::atomic<bool> stop_flag_;

    // The current job being processed by the worker thread, or a default value if idle
    Job current_job_;

    // A flag to indicate whether the current job being processed by the worker thread has been completed
    std::atomic<bool> current_job_completed_;

};

class WorkerThreadProvider
{
public:
    WorkerThreadProvider() = default;
    ~WorkerThreadProvider() = default;

    /**
     * @brief : Get the number of CPU cores available on the system
     * @return uint32_t : The number of CPU cores
     */
    uint32_t GetCpuCoreCount() const;

    /**
     * @brief : Get the total number of worker threads currently managed by the provider
     * @return uint32_t : The total number of worker threads
     */
    uint32_t GetWorkerThreadCount() const;

    /**
     * @brief : Create a new worker thread that will execute the provided job function
     * @param jobFunction : The function to be executed by the worker thread
     * @return bool : True if the worker thread was successfully created, false otherwise
     */
    bool CreateWorkerThread();

    /**
     * @brief : Create multiple worker threads that will execute the provided job function
     * @param count : The number of worker threads to create
     * @return bool : True if all worker threads were successfully created, false otherwise
     */
    bool CreateWorkerThreads(uint32_t count);

    /**
     * @brief : Get an idle worker thread from the pool
     * @return WorkerThread* : A pointer to an idle worker thread, or nullptr if no idle threads are available
     */
    WorkerThread* GetIdleWorkerThread();

    /**
     * @brief : Get a list of all currently idle worker threads
     * @return std::vector<const WorkerThread*> : A vector of pointers to the currently idle worker threads
     * @note : The returned worker threads should not be modified by the caller
     */
    std::vector<const WorkerThread*> GetIdleWorkerThreads() const;

    /**
     * @brief : Get a list of all currently running worker threads
     * @return std::vector<const WorkerThread*> : A vector of pointers to the currently running worker threads
     * @note : The returned worker threads should not be modified by the caller
     */
    std::vector<const WorkerThread*> GetRunningWorkerThreads() const;

private:
    // A vector that holds all the worker threads managed by the provider
    std::vector<WorkerThread> worker_threads_;

    // A vector that holds pointers to the currently running worker threads
    std::vector<WorkerThread*> running_threads_;

    // A vector that holds pointers to the currently idle worker threads
    std::vector<WorkerThread*> idle_threads_;

};

class JobScheduler
{
public:
    /**
     * @brief : Construct a new JobScheduler object with the specified worker thread provider
     */
    JobScheduler(WorkerThreadProvider& worker_thread_provider);

    ~JobScheduler() = default;

    /**
     * @brief : Schedule a job to be executed by an idle worker thread
     * @param job : The job to be scheduled
     * @return JobState : The state of the scheduled job (e.g., pending, running, completed)
     */
    JobState ScheduleJob(Job job);

private:
    // A reference to the worker thread provider that manages the worker threads used for executing jobs
    WorkerThreadProvider& worker_thread_provider_;

    // A queue that holds the jobs that are waiting to be processed by worker threads
    std::queue<Job> job_queue_;

    // A condition variable to signal worker threads when new jobs are added to the queue
    std::condition_variable condition_;

    // A mutex to protect access to the job queue and condition variable
    std::mutex mutex_;

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
     * @brief : Construct a new JobTracker object with the specified worker thread provider
     */
    JobTracker(WorkerThreadProvider& worker_thread_provider);

    ~JobTracker() = default;

    /**
     * @brief : Get a list of all currently running job execution information
     * @return std::vector<JobExecutionInfo> : A vector of JobExecutionInfo objects representing the currently running jobs
     */
    std::vector<JobExecutionInfo> GetRunningJobInfos() const;

private:
    // A reference to the worker thread provider that manages the worker threads used for executing jobs
    WorkerThreadProvider& worker_thread_provider_;

};

} // namespace tecs