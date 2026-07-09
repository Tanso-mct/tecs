// Google Test
#include <gtest/gtest.h>

// TECS
#include "tecs/tecs.h"

namespace tecs_debug_test
{

struct Container
{
    int value;
};

class TestWritePort : public tecs::Port
{
public:
    TestWritePort(Container& container) : container_(container) {}

    void SetValue(int value)
    {
        container_.value = value;
    }

private:
    Container& container_;
};

class TestReadPort : public tecs::Port
{
public:
    TestReadPort(Container& container) : container_(container) {}

    int GetValue() const
    {
        return container_.value;
    }

private:
    Container& container_;
};

class TestTask : public tecs::Task
{
public:
    TestTask(int value) :
        value_(value),
        tecs::Task(tecs::TaskInfo("TestTask", 1))
    {
    }

    tecs::TaskResult Execute(tecs::Port& write_port, tecs::Port& read_port) override
    {
        // Cast the ports to the specific types
        TestWritePort* specific_write_port = write_port.ConvertToWrite<TestWritePort>();
        const TestReadPort* specific_read_port = read_port.ConvertToRead<TestReadPort>();
        EXPECT_NE(specific_write_port, nullptr);
        EXPECT_NE(specific_read_port, nullptr);

        // Set the value using the write port
        specific_write_port->SetValue(value_);

        // Wait for a moment to simulate some processing time
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        return tecs::TaskResult(true);
    }

private:
    int value_;

};

class TestSystem : public tecs::SystemBase<TestSystem>
{
public:
    TestSystem() : tecs::SystemBase<TestSystem>("Test System")
    {
        std::unique_ptr<tecs::Port> write_port = std::make_unique<TestWritePort>(container_);
        std::unique_ptr<tecs::Port> read_port = std::make_unique<TestReadPort>(container_);
        tecs::System::Initialize(std::move(write_port), std::move(read_port));
    }

private:
    Container container_{0};

};

class TestComponentConfig : public tecs::ComponentConfig
{
};

class TestComponent : public tecs::ComponentBase<TestComponent>
{
public:
    TestComponent() : tecs::ComponentBase<TestComponent>("TestComponent", GUID{0, 0, 0, 0}) {}

    bool Import(std::unique_ptr<tecs::ComponentConfig> config) override
    {
        return true;
    }

    std::unique_ptr<tecs::ComponentConfig> Export() const override
    {
        return std::make_unique<TestComponentConfig>();
    }

    bool Update(tecs::Actor actor) override
    {
        return true;
    }

};

class AnotherTestComponentConfig : public tecs::ComponentConfig
{
};

class AnotherTestComponent : public tecs::ComponentBase<AnotherTestComponent>
{
public:
    AnotherTestComponent() : tecs::ComponentBase<AnotherTestComponent>("AnotherTestComponent", GUID{1, 1, 1, 1}) {}

    bool Import(std::unique_ptr<tecs::ComponentConfig> config) override
    {
        return true;
    }

    std::unique_ptr<tecs::ComponentConfig> Export() const override
    {
        return std::make_unique<AnotherTestComponentConfig>();
    }

    bool Update(tecs::Actor actor) override
    {
        return true;
    }

};

} // namespace tecs_debug_test

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

TEST(tecs, system_debug_log)
{
    // Create an instance of the test system
    tecs_debug_test::TestSystem test_system;

    // Create a system view for the test system
    tecs::SystemView system_view(test_system);

    // Create system view vector
    std::vector<std::unique_ptr<tecs::SystemView>> system_views;
    system_views.push_back(system_view.Clone());

    // Create a system log exporter
    std::unique_ptr<tecs::LogExporter> system_log_exporter 
        = std::make_unique<tecs::SystemLogExporter>(std::move(system_views));

    // Create a task list
    tecs::TaskList task_list;

    // Create a task
    const int TEST_VALUE = 7;
    std::unique_ptr<tecs::Task> task = std::make_unique<tecs_debug_test::TestTask>(TEST_VALUE);

    // Add the task to the task list with caller information
    task_list.AddTask(std::move(task), tecs::TaskCallerInfo(__FILE__, __LINE__, __FUNCTION__));

    // Submit the task list to the system view for processing
    system_view.SubmitTaskList(std::move(task_list));

    // Flush the tasks in the system's task queue using other thread
    std::thread flush_thread([&test_system]() {
        bool flush_result = test_system.FlushTasks();
        EXPECT_TRUE(flush_result);
    });

    bool task_started = false;
    while (true)
    {
        if (test_system.GetCurrentTaskInfo().GetName() == "TestTask" && !task_started)
        {
            task_started = true;
            continue;
        }

        // Analyze the system log data
        bool analysis_success = system_log_exporter->Analyze();
        EXPECT_TRUE(analysis_success);

        // Sleep for a short duration to avoid busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (task_started && test_system.GetCurrentTaskInfo().GetName().empty())
            break;
    }

    // Export the system log data
    uint32_t log_size = 0;
    std::unique_ptr<uint8_t[]> log_data = system_log_exporter->Export(log_size);
    EXPECT_NE(log_data, nullptr);
    EXPECT_GT(log_size, 0);

    // Write the exported log data to a file
    std::string output_file_path = "../../../../data/system_log.csv";
    bool write_success = tecs::WriteBufferToFile(output_file_path, log_data.get(), log_size);
    EXPECT_TRUE(write_success);

    // Join the flush thread to ensure it has completed before the test ends
    flush_thread.join();
}

TEST(tecs, entity_debug_log)
{
    tecs::Registry registry;

    // Create an entity log exporter
    std::unique_ptr<tecs::LogExporter> entity_log_exporter 
        = std::make_unique<tecs::EntityLogExporter>(registry);

    // Create an entity
    tecs::Entity entity = registry.CreateEntity();
    EXPECT_TRUE(registry.CheckEntityValidity(entity));

    // Add components to the entity
    std::unique_ptr<tecs::Component> component = std::make_unique<tecs_debug_test::TestComponent>();
    EXPECT_TRUE(registry.AddComponent(entity, tecs_debug_test::TestComponent::GetRuntimeID(), std::move(component)));

    // Analyze the entity log data
    bool analysis_success = entity_log_exporter->Analyze();
    EXPECT_TRUE(analysis_success);

    // Add another component to the entity
    std::unique_ptr<tecs::Component> another_component = std::make_unique<tecs_debug_test::AnotherTestComponent>();
    EXPECT_TRUE(registry.AddComponent(entity, tecs_debug_test::AnotherTestComponent::GetRuntimeID(), std::move(another_component)));

    // Analyze the entity log data again after adding the second component
    analysis_success = entity_log_exporter->Analyze();
    EXPECT_TRUE(analysis_success);

    // Export the entity log data
    uint32_t log_size = 0;
    std::unique_ptr<uint8_t[]> log_data = entity_log_exporter->Export(log_size);
    EXPECT_NE(log_data, nullptr);
    EXPECT_GT(log_size, 0);

    // Write the exported log data to a file
    std::string output_file_path = "../../../../data/entity_log.csv";
    bool write_success = tecs::WriteBufferToFile(output_file_path, log_data.get(), log_size);
    EXPECT_TRUE(write_success);
}