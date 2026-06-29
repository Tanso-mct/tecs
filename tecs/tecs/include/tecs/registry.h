#pragma once

// STL
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <set>
#include <unordered_map>
#include <guiddef.h>

namespace tecs
{

class Entity
{
public:
    /**
     * @brief : Initializes the entity with invalid state
     */
    Entity();

    /**
     * @brief : Initializes the entity with a specific ID and generation
     * @param id : The unique identifier for the entity
     * @param gen : The generation count for the entity
     */
    Entity(uint32_t id, uint32_t gen);

    /**
     * @brief : Retrieves the unique identifier of the entity
     * @return : Returns the ID of the entity
     */
    uint32_t GetID() const;

    /**
     * @brief : Retrieves the generation count of the entity
     * @return : Returns the generation count of the entity
     */
    uint32_t GetGen() const;

    /**
     * @brief : Checks if the entity is valid
     * @return : Returns true if the entity is valid, false otherwise
     */
    bool IsValid() const;

    /**
     * @brief : Retrieves a combined representation of the entity's ID and generation
     * @return : Returns a 64-bit value representing the entity
     */
    uint64_t GetBits() const;

    /**
     * @brief : Compares this entity with another for equality
     * @param other : The other entity to compare with
     * @return : Returns true if both entities are equal, false otherwise
     */
    bool operator==(const Entity& other) const;

    /**
     * @brief : Compares this entity with another for inequality
     * @param other : The other entity to compare with
     * @return : Returns true if both entities are not equal, false otherwise
     */
    bool operator!=(const Entity& other) const;

    /**
     * @brief : Compares this entity with another for ordering
     * @param other : The other entity to compare with
     * @return : Returns true if this entity is less than the other, false otherwise
     */
    bool operator<(const Entity& other) const;

private:
    /**
     * @brief : Combines the ID, generation, and validity into a single 64-bit value
     * @param id : The unique identifier for the entity
     * @param gen : The generation count for the entity
     * @param valid : The validity flag for the entity
     * @return : Returns a 64-bit value representing the entity
     */
    uint64_t MakeBits(uint32_t id, uint32_t gen, uint32_t valid) const;

private:
    // A combined representation of the entity's ID and generation
    uint64_t bits_;

    static constexpr unsigned VALID_SHIFT = 0;

    static constexpr unsigned ID_SHIFT = 1;

    static constexpr unsigned GEN_SHIFT = 32;

    static constexpr uint64_t VALID_MASK = uint64_t{1} << VALID_SHIFT;

    static constexpr uint64_t ID_MASK = (uint64_t{1} << 31) - 1;

    static constexpr uint64_t GEN_MASK = (uint64_t{1} << 32) - 1;

};

class ComponentConfig
{
public:
    virtual ~ComponentConfig() = default;

};

class Component
{
public:
    /**
     * @brief : Constructor for the Component class
     * @param name : The name of the component
     * @param guid : The unique identifier for the component type
     */
    Component(std::string_view name, const GUID& guid);

    virtual ~Component() = default;

    /**
     * @brief : Imports the component configuration
     * @param config : A unique pointer to the component configuration
     * @return : Returns true if the import was successful, false otherwise
     */
    virtual bool Import(std::unique_ptr<ComponentConfig> config) = 0;

    /**
     * @brief : Exports the component configuration
     * @return : Returns a unique pointer to the component configuration
     */
    virtual std::unique_ptr<ComponentConfig> Export() const = 0;

    /**
     * @brief : Updates the component based on the given actor
     * @param actor : The actor to update the component with
     * @return : Returns true if the update was successful, false otherwise
     */
    virtual bool Update(Actor actor) = 0;

    /**
     * @brief : Retrieves the name of the component
     * @return : Returns the name of the component
     */
    std::string_view GetName() const;

    /**
     * @brief : Retrieves the unique identifier of the component type
     * @return : Returns the GUID of the component type
     */
    GUID GetGUID() const;

    /**
     * @brief : Retrieves the maximum runtime ID for components
     * @return : Returns the maximum runtime ID that can be assigned to components
     */
    static uint32_t GetMaxRuntimeID();

protected:
    // The maximum runtime ID for components
    static uint32_t max_runtime_id_;

private:
    // The name of the component
    const std::string name_;

    // The unique identifier for the component type
    const GUID guid_;

};

template<typename ComponentType>
class ComponentBase 
    : public Component
{
public:
    /**
     * @brief : Retrieves the runtime ID of the component type
     * @return : Returns a unique runtime ID for the component type
     */
    static uint32_t GetRuntimeID()
    {
        static uint32_t runtime_id = max_runtime_id_++;
        return runtime_id;
    }

};

class Registry
{
public:
    Registry() = default;

    ~Registry() = default;

    Registry(const Registry&) = delete;

    Registry& operator=(const Registry&) = delete;

    Registry(Registry&&) = default;

    Registry& operator=(Registry&&) = default;

    /**
     * @brief : Creates a new entity in the registry
     * @return : Returns the newly created entity
     */
    Entity CreateEntity();

    /**
     * @brief : Destroys an entity in the registry
     * @param entity : The entity to be destroyed
     * @return : Returns true if the entity was destroyed successfully, false otherwise
     */
    bool DestroyEntity(Entity entity);

    /**
     * @brief : Commits an entity to the registry, making it's components active
     * @param entity : The entity to be committed
     * @return : Returns true if the entity was committed successfully, false otherwise
     */
    bool CommitEntity(Entity entity);

    /**
     * @brief : Checks if an entity is valid in the registry
     * @param entity : The entity to check for validity
     * @return : Returns true if the entity is valid, false otherwise
     */
    bool CheckEntityValidity(Entity entity) const;

    /**
     * @brief : Adds a component to an entity in the registry
     * @param entity : The entity to which the component will be added
     * @param component : A unique pointer to the component to be added
     * @return : Returns true if the component was added successfully, false otherwise
     */
    bool AddComponent(Entity entity, std::unique_ptr<Component> component);

    /**
     * @brief : Checks if an entity has a specific component type in the registry
     * @param entity : The entity to check for the component
     * @param component_runtime_id : The runtime ID of the component type to check for
     * @return : Returns true if the entity has the component, false otherwise
     */
    bool HasComponent(Entity entity, uint32_t component_runtime_id) const;

    /**
     * @brief : Retrieves a pointer to a specific component of an entity in the registry
     * @param entity : The entity from which to retrieve the component
     * @param component_runtime_id : The runtime ID of the component type to retrieve
     * @return : Returns a pointer to the component if it exists, nullptr otherwise
     */
    Component* GetComponent(Entity entity, uint32_t component_runtime_id);

    /**
     * @brief : Retrieves a const pointer to a specific component of an entity in the registry
     * @param entity : The entity from which to retrieve the component
     * @param component_runtime_id : The runtime ID of the component type to retrieve
     * @return : Returns a const pointer to the component if it exists, nullptr otherwise
     */
    const Component* GetComponent(Entity entity, uint32_t component_runtime_id) const;

    /**
     * @brief : Retrieves a list of component types that an entity has in the registry
     * @param entity : The entity for which to retrieve the component runtime IDs
     * @return : Returns a vector of component runtime IDs that the entity has
     */
    std::vector<uint32_t> GetComponentTypes(Entity entity) const;

    /**
     * @brief : Removes a specific component from an entity in the registry
     * @param entity : The entity from which to remove the component
     * @param component_runtime_id : The runtime ID of the component type to remove
     * @return : Returns true if the component was removed successfully, false otherwise
     */
    bool RemoveComponent(Entity entity, uint32_t component_runtime_id);

    /**
     * @brief : Retrieves a set of entities that have a specific component type in the registry
     * @param component_runtime_id : The runtime ID of the component type to view
     * @return : Returns a const reference to a set of entities that have the specified component type
     */
    const std::set<Entity>& View(uint32_t component_runtime_id) const;

private:
    /**
     * @brief : Checks if an entity exists in the registry
     * @param entity : The entity to check for existence
     * @return : Returns true if the entity exists, false otherwise
     */
    bool CheckEntityExists(Entity entity) const;

private:
    // A type alias for a map that associates component runtime IDs with their corresponding components
    using ComponentMap = std::unordered_map<uint32_t, std::unique_ptr<Component>>;

    // A map that associates entities with their components
    std::unordered_map<Entity, ComponentMap> entity_to_components_;

    // A map that associates component runtime IDs with the set of entities that have that component
    std::unordered_map<uint32_t, std::set<Entity>> component_to_entities_;

    // A vector that stores all entities in the registry
    std::vector<Entity> entities_;

    // A vector that tracks the validity of entities in the registry
    std::vector<bool> entity_validity_;

    // A vector that stores entities that have been destroyed and can be reused
    std::vector<Entity> free_entities_;

};

class Actor
{
public:
    /**
     * @brief : Constructor for the Actor class
     * @param entity : The entity associated with this actor
     * @param registry : The registry that manages the components of this actor
     */
    Actor(Entity entity, Registry& registry);

    ~Actor() = default;

    /**
     * @brief : Checks if the actor is valid (has a valid entity)
     * @return : Returns true if the actor is valid, false otherwise
     */
    bool IsValid() const;

    /**
     * @brief : Adds a component to the actor
     * @param ComponentType : The type of the component to add
     * @param config : Optional configuration for the component
     * @return : Returns true if the component was added successfully, false otherwise
     */
    template<typename ComponentType> bool AddComponent(std::unique_ptr<ComponentConfig> config = nullptr);

    /**
     * @brief : Checks if the actor has a specific component
     * @param ComponentType : The type of the component to check for
     * @return : Returns true if the actor has the component, false otherwise
     */
    template<typename ComponentType> bool HasComponent() const;

    /**
     * @brief : Retrieves a pointer to a specific component of the actor
     * @param ComponentType : The type of the component to retrieve
     * @return : Returns a pointer to the component if it exists, nullptr otherwise
     */
    template<typename ComponentType> Component* GetComponent();

    /**
     * @brief : Retrieves a const pointer to a specific component of the actor
     * @param ComponentType : The type of the component to retrieve
     * @return : Returns a const pointer to the component if it exists, nullptr otherwise
     */
    template<typename ComponentType> const Component* GetComponent() const;

    /**
     * @brief : Removes a specific component from the actor
     * @param ComponentType : The type of the component to remove
     * @return : Returns true if the component was removed successfully, false otherwise
     */
    template<typename ComponentType> bool RemoveComponent();

private:
    // The reference to the registry
    Registry& registry_;

    // The entity associated with this actor
    Entity entity_;

};

} // namespace tecs