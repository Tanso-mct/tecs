#pragma once

// STL
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <queue>
#include <unordered_map>
#include <mutex>
#include <cassert>

namespace tecs
{

class Port
{
public:
    virtual ~Port() = default;

    /**
     * @brief : Convert the port to a specific writeable system port type
     * @param T : The type of system write port to convert to
     * @return T* : A pointer to the converted writeable system port, or nullptr if the conversion fails
     */
    template<typename T> T* ConvertToWrite() const
    {
        T* converted_port = dynamic_cast<T*>(const_cast<Port*>(this));
        assert(converted_port != nullptr && "Port conversion failed. Ensure that the port type is correct.");
        return converted_port;
    }

    /**
     * @brief : Convert the port to a specific readable system port type
     * @param T : The type of system read port to convert to
     * @return const T* : A pointer to the converted readable system port, or nullptr if the conversion fails
     */
    template<typename T> const T* ConvertToRead() const
    {
        const T* converted_port = dynamic_cast<const T*>(this);
        assert(converted_port != nullptr && "Port conversion failed. Ensure that the port type is correct.");
        return converted_port;
    }

};

class TaskInfo
{
public:
    /**
     *  @brief : Construct a new TaskInfo object with the specified name and tag
     *  @param name : The name of the task
     *  @param tag : A unique identifier for the task
     */
    TaskInfo(std::string name, uint32_t tag);

    TaskInfo() = default;
    ~TaskInfo() = default;

    /**
     * @brief : Get the name of the task
     * @return std::string_view : The name of the task
     */
    std::string_view GetName() const;

    /**
     * @brief : Get the unique identifier of the task
     * @return uint32_t : The unique identifier of the task
     */
    uint32_t GetTag() const;

private:
    // The name of the task
    std::string name_;

    // A unique identifier for the task
    uint32_t tag_;

};

class TaskResult
{
public:
    /**
     * @brief : Construct a new TaskResult object with the specified success status and message
     * @param success : A boolean indicating whether the task was successful
     * @param message : A message providing additional information about the task result
     */
    TaskResult(bool success, std::string message = "");

    ~TaskResult() = default;

    /**
     * @brief : Get the success status of the task result
     * @return bool : True if the task was successful, false otherwise
     */
    bool operator()() const;

    /**
     * @brief : Get the message associated with the task result
     * @return std::string_view : The message associated with the task result
     */
    std::string_view GetMessage() const;

private:
    // A boolean indicating whether the task was successful
    bool success_;

    // A message providing additional information about the task result
    std::string message_;

};

class Task
{
public:
    /**
     * @brief : Construct a new Task object with the specified task information
     * @param info : The information about the task
     */
    Task(TaskInfo info);

    virtual ~Task() = default;

    /**
     * @brief : Get the information about the task
     * @return TaskInfo : The information about the task
     */
    TaskInfo GetInfo() const;

    /**
     * @brief : Execute the task using the provided write and read ports
     * @param write_port : The write port to use writeable adapters
     * @param read_port : The read port to use readable adapters
     * @return TaskResult : The result of executing the task
     */
    virtual TaskResult Execute(Port& write_port, Port& read_port) = 0;

private:
    // The information about the task
    TaskInfo info_;

};

class TaskCallerInfo
{
public:
    /**
     * @brief : Construct a new TaskCallerInfo object with the specified caller information
     * @param file : The file name of the caller
     * @param line : The line number of the caller
     * @param function : The function name of the caller
     */
    TaskCallerInfo(std::string file, int line, std::string function);

    ~TaskCallerInfo() = default;

    /**
     * @brief : Get the file name of the caller
     * @return std::string_view : The file name of the caller
     */
    std::string_view GetFile() const;

    /**
     * @brief : Get the line number of the caller
     * @return int : The line number of the caller
     */
    int GetLine() const;

    /**
     * @brief : Get the function name of the caller
     * @return std::string_view : The function name of the caller
     */
    std::string_view GetFunction() const;

private:
    // The file name of the caller
    std::string file_;

    // The line number of the caller
    int line_;

    // The function name of the caller
    std::string function_;

};

class TaskSet
{
public:
    /**
     * @brief : Construct a new TaskSet object with the specified task and caller information
     * @param task : A unique pointer to the task to be executed
     * @param caller_info : The information about the caller of the task
     */
    TaskSet(std::unique_ptr<Task> task, TaskCallerInfo caller_info);

    ~TaskSet() = default;

    TaskSet(const TaskSet&) = delete;
    TaskSet& operator=(const TaskSet&) = delete;
    TaskSet(TaskSet&&) noexcept = default;
    TaskSet& operator=(TaskSet&&) noexcept = default;

    // The task to be executed
    std::unique_ptr<Task> task;

    // The information about the caller of the task
    TaskCallerInfo caller_info;
};

class TaskList
{
public:
    TaskList() = default;

    ~TaskList() = default;

    TaskList(const TaskList&) = delete;
    TaskList& operator=(const TaskList&) = delete;
    TaskList(TaskList&&) noexcept = default;
    TaskList& operator=(TaskList&&) noexcept = default;

    /**
     * @brief : Add a task to the task list with the caller information
     * @param task : A unique pointer to the task to add
     * @param caller_info : The information about the caller of the task
     */
    void AddTask(std::unique_ptr<Task> task, TaskCallerInfo caller_info);

    /**
     * @brief : Get the list of tasks in the task list
     * @return std::vector<TaskSet>& : A vector of TaskSet
     */
    std::vector<TaskSet>& GetTasks();

private:
    // A vector of TaskSet, where each TaskSet consists of a task and the caller information
    std::vector<TaskSet> tasks_;

};

class TaskListQueue
{
public:
    TaskListQueue() = default;

    ~TaskListQueue() = default;

    /**
     * @brief : Add a task list to the queue
     * @param task_list : The task list to add to the queue
     */
    void Enqueue(TaskList&& task_list);

    /**
     * @brief : Remove and return the next task list from the queue
     * @return TaskList : The next task list from the queue
     */
    TaskList Dequeue();

private:
    // A queue of task lists
    std::queue<TaskList> queue_;

};

class TaskProcessor
{
public:
    TaskProcessor() = default;

    ~TaskProcessor() = default;

    /**
     * @brief : Process the tasks in the task queue using the provided write and read ports
     * @param task_queue : The task queue to process tasks from
     * @param write_port : The write port to use writeable adapters
     * @param read_port : The read port to use readable adapters
     */
    bool FlushTasks(TaskListQueue& task_queue, Port& write_port, Port& read_port);

    /**
     * @brief : Get the information about the currently executing task
     * @return TaskInfo : The information about the currently executing task
     */
    TaskInfo GetCurrentTaskInfo() const;

private:
    // A mutex to protect access to the current task information
    mutable std::mutex mutex_;

    // The information about the currently executing task
    TaskInfo current_task_info_;

};

class System
{
public:
    /**
     * @brief : Create a new task processor and task list queue for the system
     */
    System();

    virtual ~System() = default;

    /**
     * @brief : Submit a task list to the system for processing
     * @param task_list : The task list to submit to the system
     */
    void SubmitTaskList(TaskList&& task_list);

    /**
     * @brief : Get a reference to the read port for the system
     * @return const Port& : A reference to the read port for the system
     */
    const Port& GetReadPort() const;

    /**
     * @brief : Flush the tasks in the system's task queue using the task processor
     * @return bool : True if the tasks were successfully flushed, false otherwise
     */
    bool FlushTasks();

    /**
     * @brief : Get the information about the currently executing task in the system
     * @return TaskInfo : The information about the currently executing task in the system
     */
    TaskInfo GetCurrentTaskInfo() const;

protected:
    /**
     * @brief : Initialize the system with the provided write and read ports
     * @param write_port : A unique pointer to the write port to use writeable adapters
     * @param read_port : A unique pointer to the read port to use readable adapters
     * @return bool : True if the system was successfully initialized, false otherwise
     */
    bool Initialize(std::unique_ptr<Port> write_port, std::unique_ptr<Port> read_port);

    // Static variable to keep track of the next available system ID
    static uint32_t next_id_;

private:
    // The task processor for the system
    TaskProcessor task_processor_;

    // The task list queue for the system
    TaskListQueue task_queue_;

    // A unique pointer to the write port for the system
    std::unique_ptr<Port> write_port_;

    // A unique pointer to the read port for the system
    std::unique_ptr<Port> read_port_;
};

template <typename T>
class SystemBase
    : public System
{
public:
    virtual ~SystemBase() = default;

    /**
     * @brief : Get the unique identifier of the system
     * @return uint32_t : The unique identifier of the system
     */
    static uint32_t GetID()
    {
        static uint32_t id = next_id_++;
        return id;
    }

};

class SystemView
{
public:
    /**
     * @brief : Construct a new SystemView object for the specified system
     * @param system : The system to create a view for
     */
    SystemView(System& system);

    ~SystemView() = default;

    /**
     * @brief : Submit a task list to the system view for processing
     * @param task_list : The task list to submit to the system view
     */
    void SubmitTaskList(TaskList&& task_list);

    /**
     * @brief : Get a reference to the read port for the system view
     * @return const Port& : A reference to the read port for the system view
     */
    const Port& GetReadPort() const;

    /**
     * @brief : Flush the tasks in the system view's task queue using the task processor
     * @return bool : True if the tasks were successfully flushed, false otherwise
     */
    TaskInfo GetCurrentTaskInfo() const;

    /**
     * @brief : Create a new instance of the system view that is a copy of the current instance
     * @return std::unique_ptr<SystemView> : A unique pointer to the cloned system view
     */
    std::unique_ptr<SystemView> Clone() const;

private:
    // A reference to the system that this view represents
    System& system_;

};

class SystemViewRegistry
{
public:
    SystemViewRegistry() = default;

    ~SystemViewRegistry() = default;

    /**
     * @brief : Register a system view with the registry for a specific system ID
     * @param system_id : The unique identifier of the system to register the view for
     * @param view : A unique pointer to the system view to register
     */
    void RegisterView(uint32_t system_id, std::unique_ptr<SystemView> view);

    /**
     * @brief : Get the system view registered for a specific system ID from the registry
     * @param system_id : The unique identifier of the system to get the view for
     * @return std::unique_ptr<SystemView> : A unique pointer to the system view registered for the specified system ID
     */
    std::unique_ptr<SystemView> GetView(uint32_t system_id) const;

private:
    // A map of system IDs to their corresponding registered system views
    std::unordered_map<uint32_t, std::unique_ptr<SystemView>> views_;

};

} // namespace tecs

