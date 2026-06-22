// Google Test
#include <gtest/gtest.h>

// TECS
#include "tecs/tecs.h"

// STL
#include <thread>
#include <chrono>

TEST(tecs, schedule_job)
{
    // Create job scheduler
    tecs::JobScheduler job_scheduler;

    // Create a job that sets a result variable to a specific value
    int result = 0;
    const int EXPECTED_RESULT = 7;
    tecs::Job job(tecs::JobType("TestJob", 1), [&result, EXPECTED_RESULT]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Simulate some delay
        result = EXPECTED_RESULT;
    });

    // Schedule the job
    tecs::JobState job_state = job_scheduler.ScheduleJob(std::move(job));

    // Wait for the job to complete
    job_state.WaitForCompletion();

    // Verify that the result variable has been set to the expected value
    EXPECT_EQ(result, EXPECTED_RESULT);
}

TEST(tecs, schedule_multi_job)
{
    // Create job scheduler
    tecs::JobScheduler job_scheduler;

    // Create first job that sets a result variable to a specific value
    int result1 = 0;
    const int EXPECTED_RESULT1 = 7;
    tecs::Job job1(tecs::JobType("TestJob1", 1), [&result1, EXPECTED_RESULT1]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Simulate some delay
        result1 = EXPECTED_RESULT1;
    });

    // Create second job that sets a different result variable to a specific value
    int result2 = 0;
    const int EXPECTED_RESULT2 = 11;
    tecs::Job job2(tecs::JobType("TestJob2", 2), [&result2, EXPECTED_RESULT2]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(300)); // Simulate some delay
        result2 = EXPECTED_RESULT2;
    });

    // Create third job that sets a different result variable to a specific value
    int result3 = 0;
    const int EXPECTED_RESULT3 = 13;
    tecs::Job job3(tecs::JobType("TestJob3", 3), [&result3, EXPECTED_RESULT3]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate some delay
        result3 = EXPECTED_RESULT3;
    });

    // Schedule the jobs
    tecs::JobState job_state1 = job_scheduler.ScheduleJob(std::move(job1));
    tecs::JobState job_state2 = job_scheduler.ScheduleJob(std::move(job2));
    tecs::JobState job_state3 = job_scheduler.ScheduleJob(std::move(job3));

    while (!job_state1.IsCompleted() || !job_state2.IsCompleted() || !job_state3.IsCompleted())
    {
        // Output the current status of each job
        std::cout << "Job 1 completed: " << job_state1.IsCompleted();
        std::cout << ", Job 2 completed: " << job_state2.IsCompleted();
        std::cout << ", Job 3 completed: " << job_state3.IsCompleted() << std::endl;

        // Wait for all jobs to complete
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Output the final status of each job
    std::cout << "Final status - Job 1 completed: " << job_state1.IsCompleted();
    std::cout << ", Job 2 completed: " << job_state2.IsCompleted();
    std::cout << ", Job 3 completed: " << job_state3.IsCompleted() << std::endl;

    // Verify that the result variables have been set to the expected values
    EXPECT_EQ(result1, EXPECTED_RESULT1);
    EXPECT_EQ(result2, EXPECTED_RESULT2);
    EXPECT_EQ(result3, EXPECTED_RESULT3);    
}

TEST(tecs, job_track)
{
    // Create job scheduler
    tecs::JobScheduler job_scheduler;

    // Create job tracker
    tecs::JobTracker job_tracker(job_scheduler.GetWorkerThreads());

    // Create first job that sets a result variable to a specific value
    int result1 = 0;
    const int EXPECTED_RESULT1 = 7;
    tecs::Job job1(tecs::JobType("TestJob1", 1), [&result1, EXPECTED_RESULT1]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Simulate some delay
        result1 = EXPECTED_RESULT1;
    });

    // Create second job that sets a different result variable to a specific value
    int result2 = 0;
    const int EXPECTED_RESULT2 = 11;
    tecs::Job job2(tecs::JobType("TestJob2", 2), [&result2, EXPECTED_RESULT2]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(300)); // Simulate some delay
        result2 = EXPECTED_RESULT2;
    });

    // Schedule the jobs
    tecs::JobState job_state1 = job_scheduler.ScheduleJob(std::move(job1));
    tecs::JobState job_state2 = job_scheduler.ScheduleJob(std::move(job2));

    while (!job_state1.IsCompleted() || !job_state2.IsCompleted())
    {
        // Get the currently running job execution information
        std::vector<tecs::JobExecutionInfo> running_jobs = job_tracker.GetRunningJobInfos();

        // Output the currently running jobs
        if (!running_jobs.empty())
        {
            std::cout << "Currently running jobs:";
            for (const auto& job_info : running_jobs)
            {
                std::cout << " [Worker Thread ID: " << job_info.GetWorkerThreadID();
                std::cout << ", Job Type: " << job_info.GetJobType().GetName() << "]";
            }
            std::cout << std::endl;
        }
        else
        {
            std::cout << "No jobs are currently running." << std::endl;
        }

        // Wait for all jobs to complete
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Output the final status of each job
    std::cout << "Final status - Job 1 completed: " << job_state1.IsCompleted();
    std::cout << ", Job 2 completed: " << job_state2.IsCompleted() << std::endl;

    // Verify that the result variables have been set to the expected values
    EXPECT_EQ(result1, EXPECTED_RESULT1);
    EXPECT_EQ(result2, EXPECTED_RESULT2);
}