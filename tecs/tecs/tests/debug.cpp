// Google Test
#include <gtest/gtest.h>

// TECS
#include "tecs/tecs.h"

TEST(tecs, job_debug_log)
{
    // Create job scheduler
    tecs::JobScheduler job_scheduler;

    // Create job tracker
    tecs::JobTracker job_tracker(job_scheduler.GetWorkerThreads());

    // Create JobLogExporter
    std::unique_ptr<tecs::LogExporter> job_log_exporter = std::make_unique<tecs::JobLogExporter>(job_tracker);

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
        // Analyze the job log data
        bool analysis_success = job_log_exporter->Analyze();
        EXPECT_TRUE(analysis_success);

        // Sleep for a short duration to avoid busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Export the job log data
    uint32_t log_size = 0;
    std::unique_ptr<uint8_t[]> log_data = job_log_exporter->Export(log_size);
    EXPECT_NE(log_data, nullptr);
    EXPECT_GT(log_size, 0);

    // Write the exported log data to a file
    std::string output_file_path = "../../../../data/job_log.csv";
    bool write_success = tecs::WriteBufferToFile(output_file_path, log_data.get(), log_size);
    EXPECT_TRUE(write_success);
}