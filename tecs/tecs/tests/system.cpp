// Google Test
#include <gtest/gtest.h>

// TECS
#include "tecs/tecs.h"

namespace tecs_system_test
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

class AnotherTestSystem : public tecs::SystemBase<AnotherTestSystem>
{
public:
    AnotherTestSystem() : tecs::SystemBase<AnotherTestSystem>("Another Test System")
    {
        std::unique_ptr<tecs::Port> write_port = std::make_unique<TestWritePort>(container_);
        std::unique_ptr<tecs::Port> read_port = std::make_unique<TestReadPort>(container_);
        tecs::System::Initialize(std::move(write_port), std::move(read_port));
    }

private:
    Container container_{0};

};

} // namespace tecs_system_test

TEST(tecs, system_port)
{
    class ReadPort : public tecs::Port
    {
    public:
        bool ReadTemp() const
        {
            return true;
        }

    };

    class WritePort : public tecs::Port
    {
    public:
        bool WriteTemp()
        {
            return true;
        }
    };

    // Create instances of the read and write ports
    std::unique_ptr<tecs::Port> read_port = std::make_unique<ReadPort>();
    std::unique_ptr<tecs::Port> write_port = std::make_unique<WritePort>();

    // Cast the read port to the specific type and call the method
    const ReadPort* specific_read_port = read_port->ConvertToRead<ReadPort>();
    ASSERT_NE(specific_read_port, nullptr);
    EXPECT_TRUE(specific_read_port->ReadTemp());

    // Cast the write port to the specific type and call the method
    WritePort* specific_write_port = write_port->ConvertToWrite<WritePort>();
    ASSERT_NE(specific_write_port, nullptr);
    EXPECT_TRUE(specific_write_port->WriteTemp());
}

TEST(tecs, task)
{
    // Create a temporary container to hold the value
    tecs_system_test::Container container{0};

    // Create instances of the read and write ports
    std::unique_ptr<tecs::Port> read_port = std::make_unique<tecs_system_test::TestReadPort>(container);
    std::unique_ptr<tecs::Port> write_port = std::make_unique<tecs_system_test::TestWritePort>(container);

    // Create an instance of the test task
    const int TEST_VALUE = 7;
    std::unique_ptr<tecs::Task> task = std::make_unique<tecs_system_test::TestTask>(TEST_VALUE);

    // Execute the task using the write and read ports
    tecs::TaskResult result = task->Execute(*write_port, *read_port);
    EXPECT_TRUE(result());
}

TEST(tecs, task_list)
{
    // Create a task list queue
    tecs::TaskListQueue task_list_queue;

    // Create a task list
    tecs::TaskList task_list;

    // Create a task
    const int TEST_VALUE = 7;
    std::unique_ptr<tecs::Task> task = std::make_unique<tecs_system_test::TestTask>(TEST_VALUE);

    // Add the task to the task list with caller information
    task_list.AddTask(std::move(task), tecs::TaskCallerInfo(__FILE__, __LINE__, __FUNCTION__));

    // Enqueue the task list to the task list queue
    task_list_queue.Enqueue(std::move(task_list));

    // Dequeue the task list from the task list queue
    tecs::TaskList dequeued_task_list = task_list_queue.Dequeue();
    EXPECT_EQ(dequeued_task_list.GetTasks().size(), 1);
    EXPECT_EQ(dequeued_task_list.GetTasks()[0].task->GetInfo().GetName(), "TestTask");
    EXPECT_EQ(dequeued_task_list.GetTasks()[0].task->GetInfo().GetTag(), 1);
}

TEST(tecs, task_processor)
{
    // Create a temporary container to hold the value
    tecs_system_test::Container container{0};

    // Create instances of the read and write ports
    std::unique_ptr<tecs::Port> read_port = std::make_unique<tecs_system_test::TestReadPort>(container);
    std::unique_ptr<tecs::Port> write_port = std::make_unique<tecs_system_test::TestWritePort>(container);

    // Create a task list queue
    tecs::TaskListQueue task_list_queue;

    // Create a task list
    tecs::TaskList task_list;

    // Create a task
    const int TEST_VALUE = 7;
    std::unique_ptr<tecs::Task> task = std::make_unique<tecs_system_test::TestTask>(TEST_VALUE);

    // Add the task to the task list with caller information
    task_list.AddTask(std::move(task), tecs::TaskCallerInfo(__FILE__, __LINE__, __FUNCTION__));

    // Enqueue the task list to the task list queue
    task_list_queue.Enqueue(std::move(task_list));

    // Create a task processor
    tecs::TaskProcessor task_processor;

    // Flush the tasks in the task queue using the write and read ports
    bool flush_result = task_processor.FlushTasks(task_list_queue, *write_port, *read_port);
    EXPECT_TRUE(flush_result);

    // Verify that the value in the container has been updated by the executed task
    EXPECT_EQ(container.value, TEST_VALUE);
}

TEST(tecs, system)
{
    // Create an instance of the test system
    tecs_system_test::TestSystem test_system;

    // Create a task list
    tecs::TaskList task_list;

    // Create a task
    const int TEST_VALUE = 7;
    std::unique_ptr<tecs::Task> task = std::make_unique<tecs_system_test::TestTask>(TEST_VALUE);

    // Add the task to the task list with caller information
    task_list.AddTask(std::move(task), tecs::TaskCallerInfo(__FILE__, __LINE__, __FUNCTION__));

    // Submit the task list to the system for processing
    test_system.SubmitTaskList(std::move(task_list));

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
        
        std::cout << "Waiting for task to complete..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (task_started && test_system.GetCurrentTaskInfo().GetName().empty())
            break;
    }

    // Verify that the value in the container has been updated by the executed task
    const tecs_system_test::TestReadPort* read_port 
        = test_system.GetReadPort().ConvertToRead<tecs_system_test::TestReadPort>();
    ASSERT_NE(read_port, nullptr);
    EXPECT_EQ(read_port->GetValue(), TEST_VALUE);

    // Join the flush thread to ensure it has completed before the test ends
    flush_thread.join();
}

TEST(tecs, system_base)
{
    // Get the unique identifiers for the test system and another test system
    uint32_t test_system_id = tecs_system_test::TestSystem::GetID();
    uint32_t another_test_system_id = tecs_system_test::AnotherTestSystem::GetID();

    // Output the unique identifiers for debugging purposes
    std::cout << "TestSystem ID: " << test_system_id << std::endl;
    std::cout << "AnotherTestSystem ID: " << another_test_system_id << std::endl;

    // Verify that the unique identifiers are different
    EXPECT_NE(test_system_id, another_test_system_id);
}

TEST(tecs, system_view)
{
    // Create an instance of the test system
    tecs_system_test::TestSystem test_system;

    // Create a system view for the test system
    tecs::SystemView system_view(test_system);

    // Create a task list
    tecs::TaskList task_list;

    // Create a task
    const int TEST_VALUE = 7;
    std::unique_ptr<tecs::Task> task = std::make_unique<tecs_system_test::TestTask>(TEST_VALUE);

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
        
        std::cout << "Waiting for task to complete..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (task_started && test_system.GetCurrentTaskInfo().GetName().empty())
            break;
    }

    // Verify that the value in the container has been updated by the executed task
    const tecs_system_test::TestReadPort* read_port 
        = system_view.GetReadPort().ConvertToRead<tecs_system_test::TestReadPort>();
    ASSERT_NE(read_port, nullptr);
    EXPECT_EQ(read_port->GetValue(), TEST_VALUE);

    // Join the flush thread to ensure it has completed before the test ends
    flush_thread.join();
}

TEST(tecs, system_view_list)
{
    // Create an instance of the test system
    tecs_system_test::TestSystem test_system;

    // Create a system view for the test system
    std::unique_ptr<tecs::SystemView> system_view = std::make_unique<tecs::SystemView>(test_system);

    // Get the unique identifier for the test system
    uint32_t test_system_id = tecs_system_test::TestSystem::GetID();

    // Create a system view registry
    tecs::SystemViewList registry;

    // Register the system view with the registry for the test system ID
    registry.AddView(test_system_id, std::move(system_view));

    // Retrieve the registered system view from the registry using the test system ID
    std::unique_ptr<tecs::SystemView> retrieved_view = registry.GetView(test_system_id);

    // Verify that the retrieved view is not null and is a valid pointer
    ASSERT_NE(retrieved_view, nullptr);
}

