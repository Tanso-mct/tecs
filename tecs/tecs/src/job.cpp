#include "pch.h"
#include "tecs/job.h"

namespace tecs
{

JobType::JobType(std::string name, uint32_t id) :
    name_(std::move(name)),
    id_(id)
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

JobState::JobState(std::shared_future<void> completion) :
    completion_(std::move(completion))
{
    assert(completion_.valid() && "Completion future must be valid");
}

bool JobState::IsCompleted() const
{
    assert(completion_.valid() && "Completion future must be valid");
    return completion_.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

bool JobState::WaitForCompletion() const
{
    assert(completion_.valid() && "Completion future must be valid");
    completion_.wait();
    return IsCompleted();
}

Job::Job(JobType type, std::function<void()> job_func) :
    type_(std::move(type)),
    job_fun_(std::move(job_func))
{
}

JobState Job::GetState() const
{
    // Ensure the shared future is valid before returning the JobState
    if (!shared_.valid()) 
        shared_ = promise_->get_future().share(); // Create a shared future from the promise

    return JobState(shared_);
}

JobType Job::GetType() const
{
    return type_;
}

void Job::Execute()
{
    assert(job_fun_ != nullptr && "Job function must not be nullptr");
    job_fun_(); // Execute the job's function
    promise_->set_value(); // Fulfill the promise to signal job completion
}

ThreadController::ThreadController() :
    stop_signal_(false)
{
}

bool ThreadController::HasStopSignal() const
{
    return stop_signal_.load();
}

void ThreadController::SignalStop()
{
    stop_signal_.store(true);
    Notify(); // Notify the condition variable to wake up the thread
}

void ThreadController::Wait(std::function<void()> predicate)
{
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, predicate); // Wait for the condition to be met
}

void ThreadController::Notify()
{
    condition_.notify_one(); // Notify the condition variable to wake up the thread
}

uint32_t WorkerThread::next_id_ = 0; // Initialize the static variable for unique thread IDs

WorkerThread::WorkerThread(std::function<void(ThreadController&, std::atomic<JobType>&)> worker_loop_func) :
    id_(next_id_++),
    current_job_type_(JobType("Idle", 0)),
    thread_controller_()
{
    // Start the worker thread and run the provided worker loop function
    thread_ = std::thread([this, worker_loop_func]()
    {
        worker_loop_func(thread_controller_, current_job_type_);
    });
}

WorkerThread::~WorkerThread()
{
    thread_controller_.SignalStop(); // Signal the worker thread to stop
    if (thread_.joinable())
        thread_.join(); // Join the worker thread to ensure it has finished execution
}

uint32_t WorkerThread::GetID() const
{
    return id_; // Return the unique identifier of the worker thread
}

JobType WorkerThread::GetWorkingJobType() const
{
    return current_job_type_.load(); // Return the current job type being processed
}

void JobQueue::PushJob(Job job)
{
    std::lock_guard<std::mutex> lock(mutex_); // Lock the mutex to protect access to the job queue
    job_queue_.push(std::move(job)); // Add the job to the queue
}

Job JobQueue::PopJob()
{
    std::lock_guard<std::mutex> lock(mutex_); // Lock the mutex to protect access to the job queue
    if (job_queue_.empty())
        return Job(JobType("Invalid", 0), [](){}); // Return a default-constructed job if the queue is empty

    Job job = std::move(job_queue_.front()); // Get the next job from the front of the queue
    job_queue_.pop(); // Remove the job from the queue
    return job; // Return the job to be executed
}

bool JobQueue::IsEmpty() const
{
    std::lock_guard<std::mutex> lock(mutex_); // Lock the mutex to protect access to the job queue
    return job_queue_.empty(); // Check if the job queue is empty
}

JobScheduler::JobScheduler()
{
    // Check the number of hardware threads available and create worker threads accordingly
    uint32_t num_threads = std::thread::hardware_concurrency();

    // Create worker threads and add them to the worker_threads_ vector
    for (uint32_t i = 0; i < num_threads; ++i)
    {
        worker_threads_.emplace_back([this](ThreadController& controller, std::atomic<JobType>& current_job_type)
        {
            while (!controller.HasStopSignal())
            {
                // Create predicate function to check for available jobs or stop signal
                auto predicate = [this, &controller]() { return !job_queue_.IsEmpty() || controller.HasStopSignal(); };

                // Wait for a job to be available or for the stop signal
                controller.Wait(predicate);

                if (controller.HasStopSignal())
                    break; // Exit the loop if the stop signal is set

                // Pop a job from the queue and execute it
                Job job = job_queue_.PopJob();

                // Store the current job type being processed by the worker thread
                current_job_type.store(job.GetType());

                // Execute the job
                job.Execute();

                // Reset the current job type to idle after execution
                current_job_type.store(JobType("Idle", 0));
            }
        });
    }
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