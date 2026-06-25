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

        return tecs::TaskResult(true);
    }

private:
    int value_;

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

