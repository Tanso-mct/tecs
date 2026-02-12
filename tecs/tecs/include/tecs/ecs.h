#pragma once

// STL
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <set>
#include <chrono>

// TECS
#include "tecs/reflection.h"
#include "tecs/guid.h"
#include "tecs/class_template.h"

namespace tecs
{

// Forward declarations
class EntityHandle;

/**
 * @brief
 * Class representing an entity in the ECS (Entity-Component-System) architecture
 * Entities are identified by a unique ID and a generation number
 */
class Entity
{
public:
    /**
     * @brief
     * Construct a new Entity object
     * If no parameters are provided, the entity is considered invalid
     */
    Entity();

    /**
     * @brief
     * Construct a new Entity object with the given ID and generation
     * 
     * @param id 
     * The unique identifier for the entity
     * 
     * @param generation 
     * The generation number for the entity
     */
    Entity(uint32_t id, uint32_t gen);

    /**
     * @brief
     * Destroy the Entity object. No special cleanup is required.
     */
    ~Entity() = default;

    /**
     * @brief
     * Get the ID of the entity
     */
    uint32_t GetID() const;

    /**
     * @brief
     * Get the generation of the entity
     */
    uint32_t GetGen() const;

    /**
     * @brief
     * Check if the entity is valid
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
     * Get the encoded bits representing the entity
     * 
     * @return uint64_t 
     * The encoded bits of the entity
     */
    uint64_t GetBits() const;

    /**
     * @brief
     * Equality operator for Entity
     * 
     * @param other 
     * The other entity to compare with
     * 
     * @return true 
     * If both entities are equal
     * 
     * @return false 
     * If both entities are not equal
     */
    bool operator==(const Entity& other) const;

    /**
     * @brief
     * Inequality operator for Entity
     * 
     * @param other 
     * The other entity to compare with
     * 
     * @return true 
     * If both entities are not equal
     * 
     * @return false 
     * If both entities are equal
     */
    bool operator!=(const Entity& other) const;

    /**
     * @brief
     * Less-than operator for Entity
     * 
     * @param other 
     * The other entity to compare with
     * 
     * @return true 
     * If this entity is less than the other entity
     * 
     * @return false 
     * If this entity is not less than the other entity
     */
    bool operator<(const Entity& other) const;

private:
    // id, generation, validity bit
    uint64_t bits_;

    // Bit shifts for encoding and decoding the entity information
    static constexpr unsigned VALID_SHIFT = 0;
    static constexpr unsigned ID_SHIFT = 1;
    static constexpr unsigned GEN_SHIFT   = 32;

    // Masks for extracting fields
    static constexpr uint64_t VALID_MASK = uint64_t{1} << VALID_SHIFT;
    static constexpr uint64_t ID_MASK = (uint64_t{1} << 31) - 1; // 31bit
    static constexpr uint64_t GEN_MASK = (uint64_t{1} << 32) - 1; // 32bit

    /**
     * @brief
     * Create the encoded bits for the entity
     * 
     * @param id 
     * The unique identifier for the entity
     * 
     * @param gen 
     * The generation number for the entity
     * 
     * @param valid 
     * The validity of the entity
     * 
     * @return uint64_t 
     * The encoded bits representing the entity
     */
    uint64_t MakeBits(size_t id, size_t gen, bool valid);
};

} // namespace tecs

namespace std
{
    /**
     * @brief
     * Specialization of std::hash for tecs::Entity
     * Allows using Entity as a key in unordered containers
     */
    template <>
    struct hash<tecs::Entity>
    {
        size_t operator()(const tecs::Entity& entity) const noexcept
        {
            return static_cast<size_t>(entity.GetBits());
        }
    };
} // namespace std

namespace tecs
{

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
     * Create an EntityHandle for the given entity.
     * 
     * @param entity
     * The entity for which the handle will be created
     * 
     * @return EntityHandle
     * The created EntityHandle
     */
    EntityHandle CreateEntityHandle(Entity entity);

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

private:
    // Referenced the world
    World& world_;

    // Referenced entity
    Entity entity_;
};

/**
 * @brief
 * Base class for all entity objects in the ECS architecture
 * Entity objects encapsulate behavior and data for specific entities
 */
class EntityObject
{
public:
    /**
     * @brief
     * Construct a new EntityObject with the given EntityHandle
     */
    EntityObject(EntityHandle entity_handle);

    /**
     * @brief
     * Destroy the EntityObject. No special cleanup is required.
     */
    virtual ~EntityObject() = default;

    /**
     * @brief
     * Get the EntityHandle associated with this entity object is valid.
     * 
     * @return true
     * If the entity object is valid
     * 
     * @return false
     * If the entity object is invalid
     */
    bool IsValid() const;

    /**
     * @brief
     * Wrap the templated AddComponent method of the EntityHandle class.
     */
    template <typename ComponentType>
    bool AddComponent(std::unique_ptr<Component::Config> config)
    {
        return entity_handle_.AddComponent<ComponentType>(std::move(config));
    }

    /**
     * @brief
     * Wrap the templated RemoveComponent method of the EntityHandle class.
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
        return entity_handle_.RemoveComponent<ComponentType>();
    }

    /**
     * @brief
     * Wrap the templated HasComponent method of the EntityHandle class.
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
        return entity_handle_.HasComponent<ComponentType>();
    }

    /**
     * @brief
     * Wrap the templated GetComponent method of the EntityHandle class.
     * 
     * @return ComponentType*
     * Pointer to the component with the specified type T if found, nullptr otherwise
     */
    template <typename ComponentType>
    ComponentType* GetComponent()
    {
        return entity_handle_.GetComponent<ComponentType>();
    }

    /**
     * @brief
     * Wrap the GetHavingComponents method of the EntityHandle class.
     * 
     * @return std::vector<uint32_t>
     * Vector of component IDs that the entity has
     */
    std::vector<uint32_t> GetHavingComponents() const
    {
        return entity_handle_.GetHavingComponents();
    }

    /**
     * @brief
     * Destroy the entity object and mark it for removal
     */
    void Destroy();

    /**
     * @brief
     * Get the entity object is started
     * 
     * @return true
     * If the entity object is started
     * 
     * @return false
     * If the entity object is not started
     */
    bool IsStarted() const { return is_started_; }

    /**
     * @brief
     * Mark the entity object as started
     */
    void MarkStarted() { is_started_ = true; }

    /**
     * @brief
     * Called when the entity object is created.
     */
    virtual void OnCreate()
    {
    }

    /**
     * @brief
     * Called when the entity object is started.
     * 
     * @return true
     * If the start was successful
     * 
     * @return false
     * If the start failed
     */
    virtual bool OnStart()
    {
        return true;
    }

    /**
     * @brief
     * Called to update the entity object with the given delta time.
     * 
     * @param delta_time
     * Time elapsed since the last update
     * 
     * @return true
     * If the update will continue
     * 
     * @return false
     * If the update should stop
     */
    virtual bool OnUpdate(float delta_time)
    {
        return true;
    }

    /**
     * @brief
     * Called when the entity object is destroyed.
     */
    virtual void OnDestroy()
    {
    }

private:
    // Handle to the entity
    EntityHandle entity_handle_;

    // Flag indicating whether the entity object has been started
    bool is_started_ = false;
};

/**
 * @brief
 * Class representing a graph of entity objects in the ECS architecture
 * The graph manages multiple entity objects and their dependencies
 */
class EntityObjectGraph
{
public:
    /**
     * @brief
     * Construct a new EntityObjectGraph. No special initialization is required.
     */
    EntityObjectGraph() = default;

    /**
     * @brief
     * Destroy the EntityObjectGraph. No special cleanup is required.
     */
    ~EntityObjectGraph() = default;

    /**
     * @brief
     * Add an entity object to the graph.
     * 
     * @param entity_object
     * Unique pointer to the entity object to be added
     */
    void AddEntityObject(std::unique_ptr<EntityObject> entity_object);

    /**
     * @brief
     * Compile the entity object graph to determine update order based on dependencies.
     * 
     * @return true
     * If the compilation was successful
     * 
     * @return false
     * If the compilation failed (e.g., due to circular dependencies)
     */
    bool Compile();

    /**
     * @brief
     * Update the entity objects in the graph with the given delta time.
     * 
     * @param delta_time
     * Time elapsed since the last update
     * 
     * @return true
     * If the update will continue
     * 
     * @return false
     * If the update should stop
     */
    bool Update(float delta_time);

private:
    // List of entity objects in the graph
    std::vector<std::unique_ptr<EntityObject>> entity_objects_;

    // Update order of entity objects based on dependencies
    std::vector<size_t> update_order_;
};

class EntityObjectSpawner :
    public Singleton<EntityObjectSpawner>
{
public:
    /**
     * @brief
     * Construct a new EntityObjectSpawner with the given entity object graph and entity factory.
     * 
     * @param entity_object_graph
     * Reference to the entity object graph
     * 
     * @param entity_factory
     * Reference to the entity factory
     */
    EntityObjectSpawner(World& world, EntityObjectGraph& entity_object_graph);

    /**
     * @brief
     * Destroy the EntityObjectSpawner. No special cleanup is required.
     */
    ~EntityObjectSpawner() = default;

    template <typename EntityObjectType, typename... Args>
    EntityHandle SpawnEntityObject(Args&&... args)
    {
        // Create entity
        Entity entity = world_.CreateEntity();

        // Create entity handle
        EntityHandle entity_handle = world_.CreateEntityHandle(entity);

        // Create entity object
        std::unique_ptr<EntityObjectType> entity_object 
            = std::make_unique<EntityObjectType>(entity_handle, std::forward<Args>(args)...);

        // Call OnCreate
        entity_object->OnCreate();

        // Add entity object to graph
        entity_object_graph_.AddEntityObject(std::move(entity_object));

		return entity_handle;
    }

private:
    // Reference to the entity object graph
    EntityObjectGraph& entity_object_graph_;

    // Reference to the world
    World& world_;
};

/**
 * @brief
 * Base class for all systems in the ECS architecture
 * Systems operate on entities and their components to implement logic
 */
class System
{
public:
    /**
     * @brief
     * Construct a new System object. No special initialization is required.
     */
    System() = default;

    /**
     * @brief
     * Destroy the System object. No special cleanup is required.
     */
    ~System() = default;

    /**
     * @brief
     * Update by using entity object graph.
     * 
     * @param entity_object_graph
     * The entity object graph to be updated
     * 
     * @param world
     * The world containing entities and components
     * 
     * @return true
     * If the system update will continue
     * 
     * @return false
     * If the system update should stop
     */
    bool Update(World& world, EntityObjectGraph& entity_object_graph);

private:
    // List of component IDs that this system operates on
    // Updates are called in the order of these components.
    const std::vector<uint32_t> component_ids_;

    // Flag indicating whether this is the first update
    bool is_first_update_ = true;

    // Time point of the last update
    std::chrono::high_resolution_clock::time_point last_update_time_;
};

} // namespace tecs