#include "pch.h"
#include "tecs/job.h"

namespace tecs
{

JobType::JobType(std::string name, uint32_t id) :
    name_(std::move(name)),
    id_(id)
{
}

JobType::JobType() :
    name_(DEFAULT_JOB_TYPE_NAME),
    id_(DEFAULT_JOB_TYPE_ID)
{
}

JobType::JobType(const JobType& other)
{
    std::lock_guard<std::mutex> lock(other.mutex_); // Lock the mutex of the other JobType to safely copy its data
    name_ = other.name_; // Copy the name from the other JobType
    id_ = other.id_; // Copy the ID from the other JobType
}

bool JobType::operator==(const JobType& other) const
{
    return (name_ == other.name_) && (id_ == other.id_);
}

bool JobType::operator!=(const JobType& other) const
{
    return !(*this == other);
}

std::string_view JobType::GetName() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return name_;
}

uint32_t JobType::GetID() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return id_;
}

void JobType::Update(std::string name, uint32_t id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    name_ = std::move(name); // Update the name of the job type
    id_ = id; // Update the unique identifier of the job type
}

void JobType::Reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    name_ = DEFAULT_JOB_TYPE_NAME;
    id_ = DEFAULT_JOB_TYPE_ID;
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
    promise_ = std::make_shared<std::promise<void>>(); // Create a promise for job completion
    shared_ = promise_->get_future().share(); // Create a shared future from the promise
}

JobState Job::GetState() const
{
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

void ThreadController::Wait(std::function<bool()> predicate)
{
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, predicate); // Wait for the condition to be met
}

void ThreadController::Notify()
{
    condition_.notify_one(); // Notify the condition variable to wake up the thread
}

uint32_t WorkerThread::next_id_ = 0; // Initialize the static variable for unique thread IDs

WorkerThread::WorkerThread(std::function<void(ThreadController&, JobType&)> worker_loop_func) :
    id_(next_id_++),
    current_job_type_(JobType()),
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
    return current_job_type_;
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
        return Job(JobType(), [](){}); // Return a default-constructed job if the queue is empty

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
        worker_threads_.emplace_back(std::make_unique<WorkerThread>(
            [this](ThreadController& controller, JobType& current_job_type)
            {
                while (!controller.HasStopSignal())
                {
                    // Create predicate function to check for available jobs or stop signal
                    auto predicate 
                        = [this, &controller]() { return !job_queue_.IsEmpty() || controller.HasStopSignal(); };

                    // Wait for a job to be available or for the stop signal
                    controller.Wait(predicate);

                    if (controller.HasStopSignal())
                        break; // Exit the loop if the stop signal is set

                    // Pop a job from the queue and execute it
                    Job job = job_queue_.PopJob();

                    // Store the current job type being processed by the worker thread
                    current_job_type.Update(job.GetType().GetName().data(), job.GetType().GetID());

                    // Execute the job
                    job.Execute();

                    // Reset the current job type to idle after execution
                    current_job_type.Reset();
                }
            }));
    }
}

JobScheduler::~JobScheduler()
{
    // Clear the vector of worker threads, which will invoke their destructors
    worker_threads_.clear();
}

JobState JobScheduler::ScheduleJob(Job job)
{
    // Get the state of the job before scheduling it
    JobState job_state = job.GetState();

    // Push the job to the job queue for execution by worker threads
    job_queue_.PushJob(std::move(job));

    return job_state;
}

const std::vector<std::unique_ptr<WorkerThread>>& JobScheduler::GetWorkerThreads() const
{
    return worker_threads_; // Return a reference to the vector of worker threads
}

JobExecutionInfo::JobExecutionInfo(uint32_t worker_thread_id, JobType job_type) :
    worker_thread_id_(worker_thread_id),
    job_type_(std::move(job_type))
{
}

uint32_t JobExecutionInfo::GetWorkerThreadID() const
{
    return worker_thread_id_; // Return the ID of the worker thread processing the job
}

JobType JobExecutionInfo::GetJobType() const
{
    return job_type_; // Return the type of the job being processed
}

JobTracker::JobTracker(const std::vector<std::unique_ptr<WorkerThread>>& worker_threads) :
    worker_threads_(worker_threads)
{
}

std::vector<JobExecutionInfo> JobTracker::GetRunningJobInfos() const
{
    std::vector<JobExecutionInfo> running_jobs;

    // Iterate through the worker threads to gather information about currently running jobs
    for (const auto& worker_thread : worker_threads_)
    {
        JobType current_job_type = worker_thread->GetWorkingJobType();

        // Only include worker threads that are currently processing a job (not idle)
        if (current_job_type != JobType())
            running_jobs.emplace_back(worker_thread->GetID(), current_job_type);
    }

    return running_jobs; // Return the vector of currently running job execution information
}

} // namespace tecs