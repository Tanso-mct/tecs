#pragma once

// STL
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <set>
#include <chrono>
#include <initguid.h>

// TECS
#include "tecs/id.h"
#include "tecs/reflection.h"
#include "tecs/job.h"
#include "tecs/class_template.h"

namespace tecs
{

// Forward declarations
class EntityHandle;

// Define Entity as an alias for ID
using Entity = ID;

/**
 * @brief
 * Base class for all components in the ECS architecture
 * Components can be attached to entities to define their data and behavior
 */
class Component
{
public:
    /**
     * @brief
     * Configuration class for importing and exporting component data
     * You can derive from this class to implement custom configuration data
     */
    class Config
    {
    public:
        /**
         * @brief
         * Construct a new Config object with the given Reflection object
         * 
         * @param reflection
         * Unique pointer to the Reflection object for runtime type information
         */
        Config(std::unique_ptr<Reflection> reflection);

        /**
         * @brief
         * Destroy the Config object. No special cleanup is required.
         */
        virtual ~Config() = default;

        /**
         * @brief
        * Get the Reflection object associated with this configuration
        * 
        * @return const Reflection&
        * Reference to the Reflection object
         */
        const Reflection& GetReflection() const;

    private:
        // Reflection object for runtime type information
        std::unique_ptr<Reflection> reflection_ = nullptr;
    };

    /**
     * @brief
     * Construct a new Component object with the given name and GUID
     * 
     * @param name
     * The name of the component
     * 
     * @param guid
     * The GUID of the component
     */
    Component(std::string_view name, const GUID& guid);

    /**
     * @brief
     * Destroy the Component object. No special cleanup is required.
     */
    virtual ~Component() = default;

    /**
     * @brief
     * Import component data from the given configuration
     * 
     * @param config
     * Unique pointer to the configuration object containing the data to import
     * 
     * @return true 
     * If the import was successful
     * 
     * @return false 
     * If the import failed
     */
    virtual bool Import(std::unique_ptr<Config> config) = 0;

    /**
     * @brief
     * Export component data to a configuration object
     * 
     * @return std::unique_ptr<Config>
     * Unique pointer to the configuration object containing the exported data
     */
    virtual std::unique_ptr<Config> Export() const = 0;

    /**
     * @brief
     * Update the component with the given delta time
     * 
     * @param delta_time
     * Time elapsed since the last update
     */
    virtual void Update(EntityHandle entity_handle, float delta_time) = 0;

    /**
     * @brief
     * Get the name of the component
     * 
     * @return std::string_view 
     * The name of the component
     */
    std::string_view GetName() const { return name_; }

    /**
     * @brief
     * Get the GUID of the component
     * 
     * @return const GUID& 
     * Reference to the GUID of the component
     */
    const GUID& GetGUID() const { return guid_; }

    /**
     * @brief
     * Get the maximum assigned component ID
     * 
     * @return uint32_t
     * The maximum assigned component ID
     */
    static uint32_t GetMaxID() { return next_id_; }

protected:
    // Next unique component ID
    static uint32_t next_id_;

private:
    // Component name
    const std::string name_;

    // Component GUID
    const GUID guid_;
};

/**
 * @brief
 * Templated base class for all components in the ECS architecture
 */
template <typename T>
class ComponentBase : public Component
{
public:
    /**
     * @brief
     * Construct a new ComponentBase object with the given name and GUID
     * 
     * @param name 
     * The name of the component
     * 
     * @param guid 
     * The GUID of the component
     */
    ComponentBase(std::string_view name, const GUID& guid) :
        Component(name, guid)
    {
    }

    /**
     * @brief
     * Destroy the ComponentBase object. No special cleanup is required.
     */
    virtual ~ComponentBase() = default;

    /**
     * @brief
     * Get the unique component ID for the derived component type T
     * 
     * @return uint32_t 
     * The unique component type ID
     */
    static uint32_t ID()
    {
        static uint32_t id = next_id_++;
        return id;
    }
};

/**
 * @brief
 * Class representing the world in the ECS architecture
 * The world manages entities and their components
 */
class World
{
public:
    /**
     * @brief
     * Construct a new World object. No special initialization is required.
     */
    World() = default;

    /**
     * @brief
     * Destroy the World object. No special cleanup is required.
     */
    ~World() = default;

    // Delete copy constructor and assignment operator
    World(const World&) = delete;
    World& operator=(const World&) = delete;

    // Allow move constructor and assignment operator
    World(World&&) = default;
    World& operator=(World&&) = default;

    /**
     * @brief
     * Create a new entity in the world.
     * It only creates the entity and not commits it.
     * You need to call CommitEntity to finalize the creation.
     * 
     * @return Entity 
     * The newly created entity
     */
    Entity CreateEntity();

    /**
     * @brief
     * Commit the creation of an entity to the world.
     * After committing, the entity's components can be get from the world.
     * 
     * @param entity
     * The entity to be committed
     * 
     * @return true 
     * If the entity was successfully committed
     * 
     * @return false 
     * If the entity commit failed
     */
    bool CommitEntity(Entity entity);

    /**
     * @brief
     * Destroy an entity with their components from the world.
     * 
     * @param entity
     * The entity to be destroyed
     * 
     * @return true 
     * If the entity was successfully destroyed
     * 
     * @return false 
     * If the entity destruction failed
     */
    bool DestroyEntity(Entity entity);

    /**
     * @brief
     * Check if an entity is valid in the world.
     * 
     * @param entity
     * The entity to be checked
     * 
     * @return true 
     * If the entity is valid
     * 
     * @return false 
     * If the entity is invalid
     */
    bool CheckEntityValidity(Entity entity) const;

    /**
     * @brief
     * Add a component to an entity in the world.
     * 
     * @param entity
     * The entity to which the component will be added
     * 
     * @param component_id
     * The unique component ID
     * 
     * @param component
     * Unique pointer to the component to be added
     * 
     * @return true 
     * If the component was successfully added
     * 
     * @return false 
     * If the component addition failed
     */
    bool AddComponent(Entity entity, uint32_t component_id, std::unique_ptr<Component> component);

    /**
     * @brief
     * Templated version of AddComponent to add a component of type ComponentType to an entity.
     * 
     * @param entity
     * The entity to which the component will be added
     * 
     * @param config
     * Unique pointer to the configuration object for importing component data
     * 
     * @return true 
     * If the component was successfully added
     * 
     * @return false 
     * If the component addition failed
     */
    template <typename ComponentType>
    bool AddComponent(Entity entity, std::unique_ptr<Component::Config> config)
    {
        // Create the component
        std::unique_ptr<ComponentType> component = std::make_unique<ComponentType>();

        // Import the configuration data into the component
        bool import_result = component->Import(std::move(config));
        if (!import_result)
        {
            std::cerr << "Failed to import configuration data into component." << std::endl;
            return false; // Import failed
        }

        // Add the component to the entity
        return AddComponent(entity, ComponentType::ID(), std::move(component));
    }

    /**
     * @brief
     * Remove a component from an entity in the world.
     * 
     * @param entity
     * The entity from which the component will be removed
     * 
     * @param component_id
     * The unique component ID
     * 
     * @return true 
     * If the component was successfully removed
     * 
     * @return false 
     * If the component removal failed
     */
    bool RemoveComponent(Entity entity, uint32_t component_id);

    /**
     * @brief
     * Templated version of RemoveComponent to remove a component of type ComponentType from an entity.
     * 
     * @param entity
     * The entity from which the component will be removed
     * 
     * @return true 
     * If the component was successfully removed
     * 
     * @return false 
     * If the component removal failed
     */
    template <typename ComponentType>
    bool RemoveComponent(Entity entity)
    {
        return RemoveComponent(entity, ComponentType::ID());
    }

    /**
     * @brief
     * Check if an entity has a specific component.
     * 
     * @param entity
     * The entity to be checked
     * 
     * @param component_id
     * The unique component ID
     * 
     * @return true 
     * If the entity has the component
     * 
     * @return false 
     * If the entity does not have the component
     */
    bool HasComponent(Entity entity, uint32_t component_id) const;

    /**
     * @brief
     * Templated version of HasComponent to check if an entity has a component of type ComponentType.
     * 
     * @param entity
     * The entity to be checked
     * 
     * @return true 
     * If the entity has the component
     * 
     * @return false 
     * If the entity does not have the component
     */
    template <typename ComponentType>
    bool HasComponent(Entity entity) const
    {
        return HasComponent(entity, ComponentType::ID());
    }

    /**
     * @brief
     * Get a component of an entity in the world.
     * 
     * @param entity
     * The entity from which the component will be retrieved
     * 
     * @param component_id
     * The unique component ID
     * 
     * @return Component*
     * Pointer to the component if found, nullptr otherwise
     */
    Component* GetComponent(Entity entity, uint32_t component_id);

    /**
     * @brief
     * Templated version of GetComponent to get a component of type ComponentType from an entity.
     * 
     * @param entity
     * The entity from which the component will be retrieved
     * 
     * @return ComponentType*
     * Pointer to the component with the specified type T if found, nullptr otherwise
     */
    template <typename ComponentType>
    ComponentType* GetComponent(Entity entity)
    {
        Component* component = GetComponent(entity, ComponentType::ID());
        return dynamic_cast<ComponentType*>(component);
    }

    /**
     * @brief
     * Get a list of component IDs that the entity has.
     * 
     * @param entity
     * The entity to be queried
     * 
     * @return std::vector<uint32_t>
     * Vector of component IDs that the entity has
     */
    std::vector<uint32_t> GetHavingComponents(Entity entity) const;

    /**
     * @brief
     * Get a view of all entities that have a specific component.
     * 
     * @param component_id
     * The unique component ID
     * 
     * @return const std::set<Entity>&
     * Set of entities that have the specified component
     */
    const std::set<Entity>& View(uint32_t component_id) const;

    /**
     * @brief
     * Templated version of View to get all entities that have a component of type ComponentType.
     * 
     * @return const std::set<Entity>&
     * Set of entities that have the specified component type
     */
    template <typename ComponentType>
    const std::set<Entity>& View() const
    {
        return View(ComponentType::ID());
    }

private:
    // Map of entities to their components
    std::unordered_map<Entity, std::unordered_map<uint32_t, std::unique_ptr<Component>>> entities_to_components_map_;

    // Map of component IDs to the set of entities that have them
    // Not commited entities are not included in this map
    std::unordered_map<uint32_t, std::set<Entity>> component_to_entities_map_;

    // List of all entities in the world
    std::vector<Entity> entities_;

    // List of entity validity flags
    std::vector<bool> entity_validity_;

    // List of free entity slots for reuse
    std::vector<Entity> free_entities_;

    /**
     * @brief
     * Check if an entity meets certain conditions in the world.
     * 
     * @param entity
     * The entity to be checked
     * 
     * @return true
     * If the entity meets the conditions
     * 
     * @return false
     * If the entity does not meet the conditions
     */
    bool CheckEntityCondition(Entity entity) const;
};

/**
 * @brief
 * Handle class for managing an entity in the world
 * Provides convenient methods to interact with the entity's components
 */
class EntityHandle
{
public:
    /**
     * @brief
     * Construct a new EntityHandle object for the given entity in the world.

     */
    EntityHandle(World& world, Entity entity);

    /**
     * @brief
     * Destroy the EntityHandle object. No special cleanup is required.

     */
    ~EntityHandle() = default;

    /**
     * @brief
     * Check if the entity associated with this handle is valid.
     * 
     * @return true 
     * If the entity is valid
     * 
     * @return false 
     * If the entity is invalid
     */
    bool IsValid() const;

    /**
     * @brief
     * Commit the entity associated with this handle to the world.
     * 
     * @return true 
     * If the entity was successfully committed
     * 
     * @return false 
     * If the entity commit failed

     */
    bool Commit();

    /**
     * @brief
     * Wrap the AddComponent method of the World class for this entity.
     * 
     * @param component_id
     * The unique component ID
     * 
     * @param component
     * Unique pointer to the component to be added
     * 
     * @return true 
     * If the component was successfully added
     * 
     * @return false 
     * If the component addition failed
     */
    bool AddComponent(uint32_t component_id, std::unique_ptr<Component> component);

    /**
     * @brief
     * Wrap the templated AddComponent method of the World class for this entity.
     * 
     * @param config
     * Unique pointer to the configuration object for importing component data
     * 
     * @return true 
     * If the component was successfully added
     * 
     * @return false 
     * If the component addition failed
     */
    template <typename ComponentType>
    bool AddComponent(std::unique_ptr<Component::Config> config)
    {
        return world_.AddComponent<ComponentType>(entity_, std::move(config));
    }

    /**
     * @brief
     * Wrap the RemoveComponent method of the World class for this entity.
     * 
     * @param component_id
     * The unique component ID
     * 
     * @return true 
     * If the component was successfully removed
     * 
     * @return false 
     * If the component removal failed
     */
    bool RemoveComponent(uint32_t component_id);

    /**
     * @brief
     * Wrap the templated RemoveComponent method of the World class for this entity.
     * 
     * @return true 
     * If the component was successfully removed
     * 
     * @return false 
     * If the component removal failed
     */
    template <typename ComponentType>
    bool RemoveComponent()
    {
        return world_.RemoveComponent<ComponentType>(entity_);
    }

    /**
     * @brief
     * Wrap the HasComponent method of the World class for this entity.
     * 
     * @param component_id
     * The unique component ID
     * 
     * @return true 
     * If the entity has the component
     * 
     * @return false 
     * If the entity does not have the component
     */
    bool HasComponent(uint32_t component_id) const;

    /**
     * @brief
     * Wrap the templated HasComponent method of the World class for this entity.
     * 
     * @return true 
     * If the entity has the component
     * 
     * @return false 
     * If the entity does not have the component
     */
    template <typename ComponentType>
    bool HasComponent() const
    {
        return world_.HasComponent<ComponentType>(entity_);
    }

    /**
     * @brief
     * Wrap the GetComponent method of the World class for this entity.
     * 
     * @param component_id
     * The unique component ID
     * 
     * @return Component*
     * Pointer to the component if found, nullptr otherwise
     */
    Component* GetComponent(uint32_t component_id);

    /**
     * @brief
     * Wrap the templated GetComponent method of the World class for this entity.
     * 
     * @return ComponentType*
     * Pointer to the component with the specified type T if found, nullptr otherwise
     */
    template <typename ComponentType>
    ComponentType* GetComponent()
    {
        return world_.GetComponent<ComponentType>(entity_);
    }

    /**
     * @brief
     * Wrap the GetHavingComponents method of the World class for this entity.
     * 
     * @return std::vector<uint32_t>
     * Vector of component IDs that the entity has
     */
    std::vector<uint32_t> GetHavingComponents() const;

    /**
     * @brief
     * Wrap the DestroyEntity method of the World class for this entity.
     */
    bool Destroy();

    /**
     * @brief
     * Clone this EntityHandle, creating a new handle that references the same entity in the world
     * 
     * @return std::unique_ptr<EntityHandle>
     * Unique pointer to the cloned EntityHandle
     */
    std::unique_ptr<EntityHandle> Clone() const;

private:
    // Referenced the world
    World& world_;

    // Referenced entity
    Entity entity_;
};


/**
 * @brief 
 * Base class for all systems
 * Systems are designed to provide specific functions
 */
class System
{
public:
    /**
     * @brief
     * Interface for the context
     * It holds internally data specific to each system
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
     * Class representing a task to be executed by the system
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
         * @param ctx
         * Reference to the Context object for this system
         * 
         * @param job_sched 
         * Reference to the JobScheduler
         * 
         * @return true 
         * If the task was successful
         * 
         * @return false 
         * If the task failed
         */
        bool Execute(Context& ctx, JobScheduler& job_sched);

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
     * Construct a new System object
     * 
     * @param job_sched 
     * Reference to the JobScheduler
     * 
     * @param ctx
     * Unique pointer to the Context object for this system
     */
    System(JobScheduler& job_sched, std::unique_ptr<Context> ctx);

    /**
     * @brief Destroy the System object
     * This class is intended to be used as a base class, so the destructor is virtual
     */
    virtual ~System() = default;

    /**
     * @brief 
     * Submit a list of tasks to the system for execution
     * 
     * @param tasks
     * List of tasks to be submitted
     */
    void SumbitTaskList(TaskList tasks);

    /**
     * @brief 
     * Get the Context object
     * You can use this to access internal data of the system
     * 
     * @return 
     * Reference to the Context object
     */
    const Context& GetContext() const;

    /**
     * @brief
     * Pre-update phase of the system
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
     * Update phase of the system
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
     * Post-update phase of the system
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
 * Template base class for systems of type T
 * This class provides a static method to get the unique ID of the system type T
 */
template <typename T>
class SystemBase :
    public System
{
public:
    /**
     * @brief
     * Construct a new System Base object, No special initialization is required.
     */
    virtual ~SystemBase() = default;

    /**
     * @brief
     * Get the static ID of the system type T
     * 
     * @return uint32_t
     * The static ID of the system type T
     */
    static uint32_t ID()
    {
        static uint32_t id = next_id_++;
        return id;
    }
};

/**
 * @brief
 * System Proxy class for providing a simplified interface to interact with systems
 * This class allows you to submit tasks and access the system context without directly interacting with the System
 */
class SystemProxy
{
public:
    /**
     * @brief
     * Construct a new System Proxy object
     * 
     * @param system 
     * Reference to the System to be proxied
     */
    SystemProxy(System& system);

    /**
     * @brief
     * Wrapper for System::SumbitTaskList
     * 
     * @param tasks 
     * List of tasks to be submitted
     */
    void SumbitTaskList(System::TaskList tasks);

    /**
     * @brief
     * Wrapper for System::GetContext
     * 
     * @return 
     * Pointer to the Context object casted to the specified type T
     */
    template <typename T>
    const T* GetContext() const
    {
        return system_.GetContext().As<T>();
    }

    /**
     * @brief
     * Clone the SystemProxy
     * 
     * @return std::unique_ptr<SystemProxy>
     * Unique pointer to the cloned SystemProxy
     */
    std::unique_ptr<SystemProxy> Clone() const;

private:
    // Reference to the proxied System
    System& system_;
};

/**
 * @brief
 * System Proxy Manager class for managing multiple system proxies
 * This class is a singleton and provides a centralized way to access system proxies
 */
class SystemProxyManager :
    public Singleton<SystemProxyManager>
{
public:
    /**
     * @brief
     * Construct a new System Proxy Manager object. No special initialization is required.
     */
    SystemProxyManager() = default;

    /**
     * @brief
     * Destroy the System Proxy Manager object. No special cleanup is required.
     */
    ~SystemProxyManager() override = default;

    /**
     * @brief
     * Register a SystemProxy for a specific system type T
     * 
     * @param system_id
     * The unique ID of the system type T
     * 
     * @param proxy
     * Unique pointer to the SystemProxy to be registered
     */
    void RegisterSystemProxy(uint32_t system_id, std::unique_ptr<SystemProxy> proxy);

    /**
     * @brief
     * Template wrapper for RegisterSystemProxy to register a SystemProxy for a specific system type T
     * 
     * @param proxy
     * Unique pointer to the SystemProxy to be registered
     */
    template <typename T>
    void RegisterSystemProxy(std::unique_ptr<SystemProxy> proxy)
    {
        // Get the system ID for the specified system type T
        uint32_t system_id = SystemBase<T>::ID();

        // Register the SystemProxy for the specified system type T
        RegisterSystemProxy(system_id, std::move(proxy));
    }

    /**
     * @brief
     * Get the SystemProxy for a specific system type T
     * 
     * @return SystemProxy&
     * Reference to the SystemProxy for the specified system type T
     */
    std::unique_ptr<SystemProxy> GetSystemProxy(uint32_t system_id);

    /**
     * @brief
     * Template wrapper for GetSystemProxy to get the proxy for a specific system type T
     * 
     * @return SystemProxy&
     * Reference to the SystemProxy for the specified system type T
     */
    template <typename T>
    std::unique_ptr<SystemProxy> GetSystemProxy()
    {
        // Get the system ID for the specified system type T
        uint32_t system_id = SystemBase<T>::ID();

        return GetSystemProxy(system_id);
    }

private:
    // Internal storage for system proxies, mapped by system ID
    std::unordered_map<uint32_t, std::unique_ptr<SystemProxy>> system_proxies_;

    // Mutex for thread-safe access to the system proxies map
    std::mutex mutex_;
};

/**
 * @brief
 * Helper function to add a task to a system's task list with a specific context type
 */
template <typename ContextType>
void AddTask(
    tecs::System::TaskList& task_list, 
    std::function<bool(ContextType&, tecs::JobScheduler&)> func,
    std::unique_ptr<tecs::System::Task::Info> info = nullptr)
{
    // Create a task function that casts the context to the specified type and executes the provided function
    tecs::System::Task::Func task_func = 
        [func = std::move(func)](tecs::System::Context& ctx, tecs::JobScheduler& job_sched) -> bool
    {
        // Cast the context to the specified type
        ContextType* typed_ctx = ctx.As<ContextType>();
        if (!typed_ctx)
        {
            std::cerr << "Failed to cast system context to the specified type." << std::endl;
            return false; // Cast failed
        }

        // Execute the provided function with the typed context and job scheduler
        return func(*typed_ctx, job_sched);
    };

    // Create a new task with the created task function and provided info, and add it to the task list
    if (info) // If additional task information is provided, include it in the task
    {
        tecs::System::Task task(std::move(task_func), std::move(info));
        task_list.emplace_back(std::move(task));
    }
    else // If no additional task information is provided, create a task without info
    {
        tecs::System::Task task(std::move(task_func));
        task_list.emplace_back(std::move(task));
    }
}

} // namespace tecs