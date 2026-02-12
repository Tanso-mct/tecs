#include "pch.h"
#include "tecs/job.h"

namespace tecs
{

void JobState::MarkCompleted()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        completed_.store(true);
    }

    // Notify all waiting threads
    cv_.notify_all();
}

JobHandle::JobHandle(std::shared_ptr<JobState> state) : 
    state_(state)
{
}

void JobHandle::Wait()
{
    assert(state_ != nullptr && "JobState must not be nullptr");

    std::unique_lock<std::mutex> lock(state_->mutex_);
    state_->cv_.wait(lock, [this]() { return state_->completed_.load(); });
}

Job::Job(JobFunc func)
    : func_(std::move(func))
{
}

void Job::Execute()
{
    assert(func_ != nullptr && "Job function must not be nullptr");
    func_();
}

JobScheduler::JobScheduler(uint32_t num_worker_threads) :
    stop_flag_(false)
{
    // Start the specified number of worker threads
    for (uint32_t i = 0; i < num_worker_threads; ++i)
        worker_thread_.emplace_back(&JobScheduler::Work, this);
}

JobScheduler::~JobScheduler()
{
    // Signal the worker thread to stop
    stop_flag_.store(true);
    cv_.notify_all();

    // Join the worker threads
    for (std::thread& thread : worker_thread_)
        if (thread.joinable())
            thread.join();
}

JobHandle JobScheduler::ScheduleJob(Job job)
{
    // Create a new JobState for the scheduled job
    std::shared_ptr<JobState> state = std::make_shared<JobState>();

    {
        // Lock the job queue and condition variable
        std::lock_guard<std::mutex> lock(mutex_);

        // Add the job and its state to the queue
        job_queue_.push(std::make_unique<JobSet>(std::move(job), state));

        // Notify the worker thread of the new job
        cv_.notify_one();
    }

    // Return a JobHandle associated with the job's state
    return JobHandle(state);
}

JobScheduler::JobSet::JobSet(Job job, std::shared_ptr<JobState> state) :
    job_(std::move(job)), 
    state_(std::move(state))
{
}

void JobScheduler::Work()
{
    // Continuously process jobs until stop flag is set
    while (!stop_flag_.load())
    {
        // JobSet
        std::unique_ptr<JobSet> job_set = nullptr;

        {
            std::unique_lock<std::mutex> lock(mutex_);

            // Wait for a job to be available or for the stop flag to be set
            cv_.wait(lock, [this]() { return !job_queue_.empty() || stop_flag_.load(); });

            if (stop_flag_.load())
                break; // Exit if stop flag is set

            // Get job set from the queue
            job_set = std::move(job_queue_.front());

            // Remove the job set from the queue
            job_queue_.pop();
        }

        // Execute the job
        job_set->job_.Execute();

        // Mark the job as completed
        job_set->state_->MarkCompleted();
    }
}

} // namespace tecs