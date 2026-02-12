// Google Test
#include <gtest/gtest.h>

// TECS
#include "tecs/tecs.h"

namespace tecs::test
{

/**
 * @brief
 * Sample implementation of Service::Context for testing purposes
 */
class SampleContext : public Service::Context
{
public:
    SampleContext() = default;
    ~SampleContext() override = default;

    int sample_data_ = 0;
};

class SampleService : public Service
{
public:
    SampleService(JobScheduler& job_scheduler) :
        Service(job_scheduler, std::make_unique<SampleContext>())
    {
    }

    ~SampleService() override = default;

    bool PreUpdate() override
    {
        // Sample pre-update logic
        return true;
    }

    bool Update() override
    {
        // Dequeue all task lists
        std::vector<TaskList> all_tasks = task_list_queue_.Dequeue();

        // Execute each task in each task list
        for (auto& tasks : all_tasks)
        {
            for (auto& task : tasks)
            {
                if (!task.Execute(*context_, job_scheduler_))
                    return false; // Task execution failed
            }
        }

        return true;
    }

    bool PostUpdate() override
    {
        // Sample post-update logic
        return true;
    }

};

// Constant for modified value in tests
constexpr const int kModifiedValue = 100;

/**
 * @brief
 * Helper function to create a task that modifies the SampleContext data
 */
void ModifyContextData(tecs::Service::TaskList& task_list)
{
    task_list.emplace_back(tecs::Service::Task(
        [](tecs::Service::Context& context_arg, tecs::JobScheduler& job_scheduler_arg) -> bool
    {
        // Cast context to SampleContext
        tecs::test::SampleContext* sample_context = context_arg.As<tecs::test::SampleContext>();

        // Schedule a job to modify the context data
        tecs::JobHandle job_handle = job_scheduler_arg.ScheduleJob(tecs::Job([sample_context]()
        {
            // Modify the sample context data
            sample_context->sample_data_ = tecs::test::kModifiedValue;
        }));

        // Wait for the job to complete
        job_handle.Wait();

        return true; // Indicate success
    }));
}

} // namespace tecs::test

TEST(tecs, use_service)
{
    // Get cpu core count
    uint32_t num_cores = std::thread::hardware_concurrency();
    EXPECT_NE(num_cores, 0);

    // Create job scheduler
    tecs::JobScheduler job_scheduler(num_cores);

    // Create sample service
    tecs::test::SampleService sample_service(job_scheduler);

    // Create singleton service proxy manager
    tecs::ServiceProxyManager service_proxy_manager;

    {
        // Create service proxy for sample service
        tecs::ServiceProxy sample_proxy(sample_service);

        // Register sample service proxy
        service_proxy_manager.RegisterServiceProxy<tecs::test::SampleService>(
            std::make_unique<tecs::ServiceProxy>(sample_service));
    }

    {
        // Create task list
        tecs::Service::TaskList task_list;

        // Modify context data task
        tecs::test::ModifyContextData(task_list);

        // Get service proxy for sample service
        std::unique_ptr<tecs::ServiceProxy> sample_proxy =
            service_proxy_manager.GetServiceProxy<tecs::test::SampleService>();

        // Submit task list to the service via proxy
        sample_proxy->SumbitTaskList(std::move(task_list));
    }

    // Run service update phases
    ASSERT_TRUE(sample_service.PreUpdate());
    ASSERT_TRUE(sample_service.Update());
    ASSERT_TRUE(sample_service.PostUpdate());

    {
        // Get service proxy for sample service
        std::unique_ptr<tecs::ServiceProxy> sample_proxy =
            service_proxy_manager.GetServiceProxy<tecs::test::SampleService>();

        // Verify that the context data was modified by the task
        const tecs::test::SampleContext* context = sample_proxy->GetContext<tecs::test::SampleContext>();
        EXPECT_EQ(context->sample_data_, tecs::test::kModifiedValue);
    }
}