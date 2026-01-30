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

    // Result variable
    int result = 0;
    const int kExpectedResult = 1000000000;

    // Create job
    tecs::Job job([&result, kExpectedResult]()
    {
        // Simulate some work
        for (int i = 0; i < kExpectedResult; ++i)
            result++;

        // Indicate success
        return true;
    });

    // Start time measurement
    auto start_time = std::chrono::high_resolution_clock::now();

    // Schedule job
    tecs::JobHandle job_handle = job_scheduler.ScheduleJob(job);

    // Wait for job to complete
    job_handle.Wait();

    // Verify result
    EXPECT_EQ(result, kExpectedResult);

    // End time measurement
    auto end_time = std::chrono::high_resolution_clock::now();

    // Calculate duration
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "Job completed in " << duration << " milliseconds." << std::endl;
}

TEST(tecs, schedule_multiple_jobs)
{
    // Create job scheduler
    tecs::JobScheduler job_scheduler;

    size_t num_jobs = 3;

    // Result variable
    int result_1 = 0;
    int result_2 = 0;
    int result_3 = 0;
    const int kExpectedResult = 1000000000;

    // Create job 1
    tecs::Job job([&result_1, kExpectedResult]()
    {
        // Simulate some work
        for (int i = 0; i < kExpectedResult; ++i)
            result_1++;

        // Indicate success
        return true;
    });

    // Create job 2
    tecs::Job job2([&result_2, kExpectedResult]()
    {
        // Simulate some work
        for (int i = 0; i < kExpectedResult; ++i)
            result_2++;

        // Indicate success
        return true;
    });

    // Create job 3
    tecs::Job job3([&result_3, kExpectedResult]()
    {
        // Simulate some work
        for (int i = 0; i < kExpectedResult; ++i)
            result_3++;

        // Indicate success
        return true;
    });

    // Start time measurement
    auto start_time = std::chrono::high_resolution_clock::now();

    // Schedule job
    std::vector<tecs::JobHandle> job_handles;
    job_handles.push_back(job_scheduler.ScheduleJob(std::move(job)));
    job_handles.push_back(job_scheduler.ScheduleJob(std::move(job2)));
    job_handles.push_back(job_scheduler.ScheduleJob(std::move(job3)));

    // Wait for jobs to complete
    for (auto& handle : job_handles)
        handle.Wait();

    // Verify results
    EXPECT_EQ(result_1, kExpectedResult);
    EXPECT_EQ(result_2, kExpectedResult);
    EXPECT_EQ(result_3, kExpectedResult);

    // End time measurement
    auto end_time = std::chrono::high_resolution_clock::now();

    // Calculate duration
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "All jobs completed in " << duration << " milliseconds." << std::endl;
}