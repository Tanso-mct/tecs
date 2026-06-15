#include "pch.h"
#include "tecs/job.h"

namespace tecs
{

JobType::JobType(std::string name, uint32_t id) :
    name_(std::move(name)),
    id_(id),
{
}

std::string_view JobType::GetName() const
{
    return name_;
}

uint32_t JobType::GetID() const
{
    return id_;
}

JobState::JobState(std::atomic<bool>* is_completed, std::condition_variable* condition, std::mutex* mutex) :
    is_completed_(is_completed),
    condition_(condition),
    mutex_(mutex)
{
}

JobState::Initialize(std::atomic<bool>* is_completed, std::condition_variable* condition, std::mutex* mutex)
{
    is_completed_ = is_completed;
    condition_ = condition;
    mutex_ = mutex;
}

bool JobState::IsCompleted() const
{
    assert(is_completed_ != nullptr && "is_completed_ must not be nullptr");

    return is_completed_->load();
}

bool JobState::WaitForCompletion() const
{
    assert(is_completed_ != nullptr && "is_completed_ must not be nullptr");
    assert(condition_ != nullptr && "condition_ must not be nullptr");

    std::unique_lock<std::mutex> lock(*mutex_);
    condition_->wait(lock, [this]() { return IsCompleted(); });
}

Job::Job(JobType type, std::function<void()> job_func) :
    type_(std::move(type)),
    job_fun_(std::move(job_func))
{
}

JobType Job::GetType() const
{
    return type_;
}

std::weak_ptr<JobState> Job::GetState() const
{
    return state_;
}

void Job::Execute()
{
    assert(job_fun_ != nullptr && "Job function must not be nullptr");
    job_fun_();
}

WorkerThread::WorkerThread() :
    current_job_(JobType("Idle", 0), [](){}), // Default idle job
    current_job_completed_(true) // No job is being processed, so it's considered completed
{
    // Start the worker thread and run the Work method
    thread_ = std::thread(&WorkerThread::Work, this);
}

WorkerThread::~WorkerThread()
{
    // Signal the worker thread to stop
    stop_flag_.store(true);
    condition_.notify_all();

    // Join the worker thread
    if (thread_.joinable())
        thread_.join();
}

uint32_t WorkerThread::GetID() const
{
    return id_;
}

std::weak_ptr<JobState> WorkerThread::DispatchJob(Job job)
{
    // Check if the worker thread is currently idle
    assert(IsIdle() && "Worker thread must be idle to dispatch a new job");

    std::weak_ptr<JobState> job_state;
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // Set the current job and mark it as not completed
        current_job_ = std::move(job);
        current_job_completed_.store(false);

        // Get state of the job
        job_state = current_job_.GetState();
        assert(!job_state.expired() && "Job state must not be expired");

        // Initialize the job state
        std::shared_ptr<JobState> shared_job_state = job_state.lock();
        assert(shared_job_state != nullptr && "Shared job state must not be nullptr");
        shared_job_state->Initialize(&current_job_completed_, &condition_, &mutex_);

        // Notify the worker thread that a new job has been dispatched
        condition_.notify_one();
    }

    return job_state;
}

bool WorkerThread::IsIdle() const
{
    return current_job_completed_.load();
}

JobType WorkerThread::GetWorkingJobType() const
{
    return current_job_.GetType();
}

void WorkerThread::Work()
{
    // Continuously process jobs until stop flag is set
    while (!stop_flag_.load())
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);

            // Wait for a job to be available or for the stop flag to be set
            condition_.wait(lock, [this]() { return !current_job_completed_.load() || stop_flag_.load(); });

            if (stop_flag_.load())
                break; // Exit if stop flag is set
        }

        // Execute the job
        current_job_.Execute();

        // Mark the job as completed
        current_job_completed_.store(true);
    }
}

uint32_t WorkerThreadProvider::GetCpuCoreCount() const
{
    return std::thread::hardware_concurrency();
}

uint32_t WorkerThreadProvider::GetWorkerThreadCount() const
{
    return static_cast<uint32_t>(worker_threads_.size());
}

bool WorkerThreadProvider::CreateWorkerThread()
{
    uint32_t max_threads = GetCpuCoreCount();
    if (GetWorkerThreadCount() >= max_threads)
        return false; // Cannot create more worker threads than the number of CPU cores

    worker_threads_.emplace_back();

    // Set the thread is idle by default
    idle_threads_.push_back(&worker_threads_.back());

    return true;
}

bool WorkerThreadProvider::CreateWorkerThreads(uint32_t count)
{
    uint32_t max_threads = GetCpuCoreCount();
    if (GetWorkerThreadCount() + count > max_threads)
        return false; // Cannot create more worker threads than the number of CPU cores

    for (uint32_t i = 0; i < count; ++i)
    {
        worker_threads_.emplace_back();

        // Set the thread is idle by default
        idle_threads_.push_back(&worker_threads_.back());
    }

    return true;
}

WorkerThread* WorkerThreadProvider::GetIdleWorkerThread()
{
    if (idle_threads_.empty())
        return nullptr; // No idle worker threads available

    WorkerThread* idle_thread = idle_threads_.back(); // Get the last idle thread
    idle_threads_.pop_back(); // Remove the thread from the idle list

    running_threads_.push_back(idle_thread); // Add the thread to the running list

    return idle_thread;
}

std::vector<const WorkerThread*> WorkerThreadProvider::GetIdleWorkerThreads() const
{
    std::vector<const WorkerThread*> idle_thread_ptrs;
    for (const WorkerThread* thread : idle_threads_)
        idle_thread_ptrs.push_back(thread);
    return idle_thread_ptrs;
}

std::vector<const WorkerThread*> WorkerThreadProvider::GetRunningWorkerThreads() const
{
    std::vector<const WorkerThread*> running_thread_ptrs;
    for (const WorkerThread* thread : running_threads_)
        running_thread_ptrs.push_back(thread);
    return running_thread_ptrs;
}

JobScheduler::JobScheduler(WorkerThreadProvider& worker_thread_provider) :
    worker_thread_provider_(worker_thread_provider)
{
    // Create job that dispatches jobs to worker threads
    Job dispatch_job(JobType("DispatchJob", 0), [this]()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(mutex_);

            // Wait for a job to be available in the queue
            condition_.wait(lock, [this]() { return !job_queue_.empty(); });

            // Get the next job from the queue
            Job job = std::move(job_queue_.front());

            // Remove the job from the queue
            job_queue_.pop();

            // Wait for an idle worker thread to be available
            WorkerThread* worker_thread = nullptr;
            condition_.wait(lock, [this, &worker_thread]() 
            {
                worker_thread = worker_thread_provider_.GetIdleWorkerThread();
                return worker_thread != nullptr;
            });

            // Dispatch the job to the worker thread
            worker_thread->DispatchJob(std::move(job));

            
        }
    });
    
}



// void JobState::MarkCompleted()
// {
//     {
//         std::lock_guard<std::mutex> lock(mutex_);
//         completed_.store(true);
//     }

//     // Notify all waiting threads
//     cv_.notify_all();
// }

// JobHandle::JobHandle(std::shared_ptr<JobState> state) : 
//     state_(state)
// {
// }

// void JobHandle::Wait()
// {
//     assert(state_ != nullptr && "JobState must not be nullptr");

//     std::unique_lock<std::mutex> lock(state_->mutex_);
//     state_->cv_.wait(lock, [this]() { return state_->completed_.load(); });
// }

// Job::Job(JobFunc func)
//     : func_(std::move(func))
// {
// }

// void Job::Execute()
// {
//     assert(func_ != nullptr && "Job function must not be nullptr");
//     func_();
// }

// JobSet::JobSet(Job job, std::shared_ptr<JobState> state) :
//     job_(std::move(job)), 
//     state_(std::move(state))
// {
// }

// JobScheduler::JobScheduler(uint32_t num_worker_threads) :
//     stop_flag_(false)
// {
//     // Start the specified number of worker threads
//     for (uint32_t i = 0; i < num_worker_threads; ++i)
//         worker_threads_.emplace_back(&JobScheduler::Work, this);
// }

// JobScheduler::~JobScheduler()
// {
//     // Signal the worker thread to stop
//     stop_flag_.store(true);
//     cv_.notify_all();

//     // Join the worker threads
//     for (std::thread& thread : worker_threads_)
//         if (thread.joinable())
//             thread.join();
// }

// JobHandle JobScheduler::ScheduleJob(Job job)
// {
//     // Create a new JobState for the scheduled job
//     std::shared_ptr<JobState> state = std::make_shared<JobState>();

//     {
//         // Lock the job queue and condition variable
//         std::lock_guard<std::mutex> lock(mutex_);

//         // Add the job and its state to the queue
//         job_queue_.push(std::make_unique<JobSet>(std::move(job), state));

//         // Notify the worker thread of the new job
//         cv_.notify_one();
//     }

//     // Return a JobHandle associated with the job's state
//     return JobHandle(state);
// }

// void JobScheduler::Work()
// {
//     // Continuously process jobs until stop flag is set
//     while (!stop_flag_.load())
//     {
//         // JobSet
//         std::unique_ptr<JobSet> job_set = nullptr;

//         {
//             std::unique_lock<std::mutex> lock(mutex_);

//             // Wait for a job to be available or for the stop flag to be set
//             cv_.wait(lock, [this]() { return !job_queue_.empty() || stop_flag_.load(); });

//             if (stop_flag_.load())
//                 break; // Exit if stop flag is set

//             // Get job set from the queue
//             job_set = std::move(job_queue_.front());

//             // Remove the job set from the queue
//             job_queue_.pop();
//         }

//         // Execute the job
//         job_set->job_.Execute();

//         // Mark the job as completed
//         job_set->state_->MarkCompleted();
//     }
// }

} // namespace tecs