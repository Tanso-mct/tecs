#pragma once

// STL
#include <functional>
#include <vector>
#include <memory>
#include <queue>
#include <mutex>
#include <string>

// TECS
#include "tecs/job.h" // JobScheduler
#include "tecs/class_template.h" // Singleton

namespace tecs
{

/**
 * @brief 
 * Base class for all services
 * Services are designed to provide specific functions
 */
class Service
{
public:
    /**
     * @brief
     * Interface for the context
     * It holds internally data specific to each service
     */
    class Context
    {
    public:
        virtual ~Context() = default;

        /**
         * @brief
         * Helper method to cast Context to derived type
         */
        template <typename T>
        T* As()
        {
            return static_cast<T*>(this);
        }

        /**
         * @brief
         * Helper method to cast Context to derived type (const version)
         */
        template <typename T>
        const T* As() const
        {
            return static_cast<const T*>(this);
        }
    };

    /**
     * @brief
     * Class representing a task to be executed by the service
     * A task consists of a function and optional additional information
     */
    class Task
    {
    public:
        /**
         * @brief
         * Interface for additional task information
         * You can inherit this class to provide custom information for each task
         */
        class Info
        {
        public:
            virtual ~Info() = default;
        };

        /**
         * @brief 
         * Alias for a task function that takes a JobScheduler reference
         * 
         * @return true
         * If the task was successful
         * 
         * @return false
         * If the task failed
         */
        using Func = std::function<bool(Context&, JobScheduler&)>;

        /**
         * @brief
         * Construct a new Task object
         * 
         * @param func 
         * The function to be executed as part of the task
         * If nullptr is passed, will be asserted
         */
        Task(Func func);

        /**
         * @brief 
         * Construct a new Task object
         * 
         * @param func 
         * The function to be executed as part of the task
         * If nullptr is passed, will be asserted
         * 
         * @param info 
         * Additional task information
         * If nullptr is passed, will be asserted
         */
        Task(Func func, std::unique_ptr<Info> info);

        ~Task() = default;

        // Disable copy semantics and enable move semantics
        Task(const Task&) = delete;
        Task& operator=(const Task&) = delete;
        Task(Task&&) noexcept = default;
        Task& operator=(Task&&) noexcept = default;

        /**
         * @brief 
         * Execute the task function
         * 
         * @param job_scheduler 
         * Reference to the JobScheduler
         * 
         * @return true 
         * If the task was successful
         * 
         * @return false 
         * If the task failed
         */
        bool Execute(Context& context, JobScheduler& job_scheduler);

        /**
         * @brief 
         * Get the task information
         * 
         * @return const Info& 
         * Reference to the Info
         */
        const Info& GetInfo() const;

    private:
        // The function to be executed as part of the task
        Func func_ = nullptr;

        // Information about the task
        std::unique_ptr<Info> info_ = nullptr;
    };

    /**
     * @brief
     * Alias for a list of tasks
     */
    using TaskList = std::vector<Task>;

    /**
     * @brief
     * Construct a new Service object
     * 
     * @param job_scheduler 
     * Reference to the JobScheduler
     * 
     * @param query 
     * Unique pointer to the Query object 
     * Can not be nullptr If nullptr is passed, will be asserted
     */
    Service(JobScheduler& job_scheduler, std::unique_ptr<Context> context);

    /**
     * @brief Destroy the Service object
     * This class is intended to be used as a base class, so the destructor is virtual
     */
    virtual ~Service() = default;

    /**
     * @brief 
     * Submit a list of tasks to the service for execution
     * 
     * @param tasks
     * List of tasks to be submitted
     */
    void SumbitTaskList(TaskList tasks);

    /**
     * @brief 
     * Get the Context object
     * You can use this to access internal data of the service
     * 
     * @return 
     * Reference to the Context object
     */
    const Context& GetContext() const;

    /**
     * @brief
     * Pre-update phase of the service
     * 
     * @return true 
     * If the pre-update was successful
     * 
     * @return false 
     * If the pre-update failed
     */
    virtual bool PreUpdate() = 0;

    /**
     * @brief
     * Update phase of the service
     * 
     * @return true 
     * If the update was successful
     * 
     * @return false 
     * If the update failed
     */
    virtual bool Update() = 0;

    /**
     * @brief
     * Post-update phase of the service
     * 
     * @return true 
     * If the post-update was successful
     * 
     * @return false 
     * If the post-update failed
     */
    virtual bool PostUpdate() = 0;

protected:
    // Static id
    static uint32_t next_id_;

    // Reference to the JobScheduler
    JobScheduler& job_scheduler_;

    // Unique pointer to the Context object
    std::unique_ptr<Context> context_ = nullptr;

    /**
     * @brief
     * Task List Queue for managing submitted tasks
     * This class is thread-safe and handles the queuing of task lists
     */
    class TaskListQueue
    {
    public:
        TaskListQueue() = default;
        ~TaskListQueue() = default;

        /**
         * @brief
         * Enqueue a list of tasks into the queue
         * 
         * @param tasks
         * List of tasks to be enqueued
         */
        void Enqueue(TaskList tasks);

        /**
         * @brief
         * Dequeue a list of tasks from the queue
         * 
         * @param tasks 
         * Reference to store the dequeued list of tasks
         * TaskList will be moved to the provided reference
         * 
         * @return true 
         * if a task list was successfully dequeued
         * 
         * @return false 
         * if the queue is empty
         */
        bool Dequeue(TaskList& tasks);

        /**
         * @brief
         * Dequeue all task lists from the queue
         * 
         * @return std::vector<TaskList>
         * Vector containing all dequeued task lists
         */
        std::vector<TaskList> Dequeue();

    private:
        // Internal storage for the task queue
        std::queue<TaskList> queue_;

        // Mutex for thread-safe access to the queue
        std::mutex mutex_;
    };

    // Task List Queue for managing submitted tasks
    TaskListQueue task_list_queue_;
};

/**
 * @brief
 * Template base class for services of type T
 * This class provides a static method to get the unique ID of the service type T
 */
template <typename T>
class ServiceBase :
    public Service
{
public:
    /**
     * @brief
     * Construct a new Service Base object, No special initialization is required.
     */
    virtual ~ServiceBase() = default;

    /**
     * @brief
     * Get the static ID of the service type T
     * 
     * @return uint32_t
     * The static ID of the service type T
     */
    static uint32_t ID()
    {
        static uint32_t id = next_id_++;
        return id;
    }
};

/**
 * @brief
 * Service Proxy class for providing a simplified interface to interact with services
 * This class allows you to submit tasks and access the service context without directly interacting with the Service
 */
class ServiceProxy
{
public:
    /**
     * @brief
     * Construct a new Service Proxy object
     * 
     * @param service 
     * Reference to the Service to be proxied
     */
    ServiceProxy(Service& service);

    /**
     * @brief
     * Wrapper for Service::SumbitTaskList
     * 
     * @param tasks 
     * List of tasks to be submitted
     */
    void SumbitTaskList(Service::TaskList tasks);

    /**
     * @brief
     * Wrapper for Service::GetContext
     * 
     * @return 
     * Pointer to the Context object casted to the specified type T
     */
    template <typename T>
    const T* GetContext() const
    {
        return service_.GetContext().As<T>();
    }

    /**
     * @brief
     * Clone the ServiceProxy
     * 
     * @return std::unique_ptr<ServiceProxy>
     * Unique pointer to the cloned ServiceProxy
     */
    std::unique_ptr<ServiceProxy> Clone() const;

private:
    // Reference to the proxied Service
    Service& service_;
};

/**
 * @brief
 * Service Proxy Manager class for managing multiple service proxies
 * This class is a singleton and provides a centralized way to access service proxies
 */
class ServiceProxyManager :
    public Singleton<ServiceProxyManager>
{
public:
    /**
     * @brief
     * Construct a new Service Proxy Manager object. No special initialization is required.
     */
    ServiceProxyManager() = default;

    /**
     * @brief
     * Destroy the Service Proxy Manager object. No special cleanup is required.
     */
    ~ServiceProxyManager() override = default;

    /**
     * @brief
     * Register a ServiceProxy for a specific service type T
     * 
     * @param service_id
     * The unique ID of the service type T
     * 
     * @param proxy
     * Unique pointer to the ServiceProxy to be registered
     */
    void RegisterServiceProxy(uint32_t service_id, std::unique_ptr<ServiceProxy> proxy);

    /**
     * @brief
     * Template wrapper for RegisterServiceProxy to register a ServiceProxy for a specific service type T
     * 
     * @param proxy
     * Unique pointer to the ServiceProxy to be registered
     */
    template <typename T>
    void RegisterServiceProxy(std::unique_ptr<ServiceProxy> proxy)
    {
        // Get the service ID for the specified service type T
        uint32_t service_id = ServiceBase<T>::ID();

        // Register the ServiceProxy for the specified service type T
        RegisterServiceProxy(service_id, std::move(proxy));
    }

    /**
     * @brief
     * Get the ServiceProxy for a specific service type T
     * 
     * @return ServiceProxy&
     * Reference to the ServiceProxy for the specified service type T
     */
    std::unique_ptr<ServiceProxy> GetServiceProxy(uint32_t service_id);

    /**
     * @brief
     * Template wrapper for GetServiceProxy to get the proxy for a specific service type T
     * 
     * @return ServiceProxy&
     * Reference to the ServiceProxy for the specified service type T
     */
    template <typename T>
    std::unique_ptr<ServiceProxy> GetServiceProxy()
    {
        // Get the service ID for the specified service type T
        uint32_t service_id = ServiceBase<T>::ID();

        return GetServiceProxy(service_id);
    }

private:
    // Internal storage for service proxies, mapped by service ID
    std::unordered_map<uint32_t, std::unique_ptr<ServiceProxy>> service_proxies_;

    // Mutex for thread-safe access to the service proxies map
    std::mutex mutex_;
};

} // namespace tecs