#include "pch.h"
#include "tecs/registry.h"

namespace tecs
{

Entity::Entity()
{
    // Initialize as invalid id
    bits_ = MakeBits(0, 0, false);
}

Entity::Entity(uint32_t id, uint32_t gen)
{
    // Initialize as valid id with given id and generation
    bits_ = MakeBits(id, gen, true);
}

uint32_t Entity::GetID() const
{
    return static_cast<std::uint32_t>((bits_ >> ID_SHIFT) & ID_MASK);
}

uint32_t Entity::GetGen() const
{
    return static_cast<std::uint32_t>((bits_ >> GEN_SHIFT) & GEN_MASK);
}

bool Entity::IsValid() const
{
    return (bits_ & VALID_MASK) != 0;
}

uint64_t Entity::GetBits() const
{
    return bits_;
}

bool Entity::operator==(const Entity& other) const
{
    return bits_ == other.bits_;
}

bool Entity::operator!=(const Entity& other) const
{
    return bits_ != other.bits_;
}

bool Entity::operator<(const Entity& other) const
{
    // Get Gens for comparison
    uint32_t this_gen = GetGen();
    uint32_t other_gen = other.GetGen();

    // Compare generations first
    if (this_gen != other_gen)
        return this_gen < other_gen;

    // Get Entitys for comparison
    uint32_t this_id = GetID();
    uint32_t other_id = other.GetID();

    // Compare Entitys if generations are equal
    return this_id < other_id;
}

uint64_t Entity::MakeBits(uint32_t id, uint32_t gen, bool valid) const
{
    uint64_t bits = 0;

    // Encode validity as 1 bit
    uint64_t v = valid ? uint64_t{1} : uint64_t{0};

    // Pack the fields into bits
    bits |= (v & uint64_t{1}) << VALID_SHIFT;
    bits |= (uint64_t(id) & ID_MASK) << ID_SHIFT;
    bits |= (uint64_t(gen) & GEN_MASK) << GEN_SHIFT;

    // Return the encoded bits
    return bits;
}

uint32_t Component::max_runtime_id_ = 0;

Component::Component(std::string_view name, const GUID& guid)
    : name_(name), guid_(guid)
{
}

std::string_view Component::GetName() const
{
    return name_;
}

GUID Component::GetGUID() const
{
    return guid_;
}

Entity Registry::CreateEntity()
{
    // Check if there are free entity slots to reuse
    if (!free_entities_.empty())
    {
        // Pop a free entity from the list
        Entity entity = free_entities_.back();
        free_entities_.pop_back();

        // Mark the entity as valid
        entity_validity_[entity.GetID()] = true;

        // Update the entity's generation
        uint32_t gen = entity.GetGen() + 1;

        // Create a new entity with updated generation
        Entity new_entity(entity.GetID(), gen);

        return new_entity; // Return the reused entity
    }

    // Create a new entity with next available ID
    uint32_t new_id = static_cast<uint32_t>(entities_.size());
    Entity new_entity(new_id, 0);

    // Add the new entity to the list
    entities_.push_back(new_entity);

    // Mark the entity as valid
    entity_validity_.push_back(true);

    return new_entity; // Return the newly created entity
}

bool Registry::DestroyEntity(Entity entity)
{
    assert(CheckEntityExists(entity) && "Entity does not exist");

    // Get component ids which the entity has
    std::vector<uint32_t> component_runtime_ids = GetComponentTypes(entity);

    // For each component id, remove the entity from the component to entities map
    for (const auto& component_runtime_id : component_runtime_ids)
        RemoveComponent(entity, component_runtime_id);

    // Mark the entity as invalid
    entity_validity_[entity.GetID()] = false;

    // Add the entity to the free entities list for reuse
    free_entities_.push_back(entity);

    return true; // Destroy successful
}

bool Registry::CommitEntity(Entity entity)
{
    assert(CheckEntityExists(entity) && "Entity does not exist");

    // Add the entity to the component to entities map for each component it has
    for (const auto& pair : entity_to_components_[entity])
    {
        uint32_t component_runtime_id = pair.first;
        component_to_entities_[component_runtime_id].insert(entity);
    }

    return true; // Commit successful
}

bool Registry::CheckEntityValidity(Entity entity) const
{
    assert(entity.GetID() < entities_.size() && "Entity ID out of range");
    return entity_validity_[entity.GetID()] && (entities_[entity.GetID()] == entity);
}

bool Registry::AddComponent(Entity entity, uint32_t component_runtime_id, std::unique_ptr<Component> component)
{
    assert(CheckEntityExists(entity) && "Entity does not exist");
    
    assert(component != nullptr && "Component pointer cannot be null");
    assert(
        entity_to_components_[entity].find(component_runtime_id) == entity_to_components_[entity].end() && 
        "Entity already has the component");

    // Add the component to the entity's component map
    entity_to_components_[entity][component_runtime_id] = std::move(component);

    // Add the entity to the component to entities map if committed
    if (component_to_entities_.find(component_runtime_id) != component_to_entities_.end())
        component_to_entities_[component_runtime_id].insert(entity);

    return true; // Component addition successful
}

bool Registry::HasComponent(Entity entity, uint32_t component_runtime_id) const
{
    assert(CheckEntityExists(entity) && "Entity does not exist");

    // Check if the entity exists in the entities to components map
    auto entity_it = entity_to_components_.find(entity);
    if (entity_it == entity_to_components_.end())
        return false; // Entity not found

    // Get the component map for the entity
    const auto& component_map = entity_it->second;

    // Check if the component exists in the entity's component map
    return component_map.find(component_runtime_id) != component_map.end();
}

Component* Registry::GetComponent(Entity entity, uint32_t component_runtime_id)
{
    assert(CheckEntityExists(entity) && "Entity does not exist");

    // Check if the entity exists in the entities to components map
    auto entity_it = entity_to_components_.find(entity);
    if (entity_it == entity_to_components_.end())
        return nullptr; // Entity not found

    // Get the component map for the entity
    auto& component_map = entity_it->second;

    // Find the component in the entity's component map
    auto component_it = component_map.find(component_runtime_id);
    if (component_it == component_map.end())
        return nullptr; // Component not found

    return component_it->second.get(); // Return pointer to the component
}

const Component* Registry::GetComponent(Entity entity, uint32_t component_runtime_id) const
{
    assert(CheckEntityExists(entity) && "Entity does not exist");

    // Check if the entity exists in the entities to components map
    auto entity_it = entity_to_components_.find(entity);
    if (entity_it == entity_to_components_.end())
        return nullptr; // Entity not found

    // Get the component map for the entity
    const auto& component_map = entity_it->second;

    // Find the component in the entity's component map
    auto component_it = component_map.find(component_runtime_id);
    if (component_it == component_map.end())
        return nullptr; // Component not found

    return component_it->second.get(); // Return pointer to the component
}

std::vector<uint32_t> Registry::GetComponentTypes(Entity entity) const
{
    // Get the component map for the entity
    const ComponentMap& component_map = entity_to_components_.at(entity);

    // Prepare a vector to hold the component types
    std::vector<uint32_t> component_types;

    // Iterate over the component map and collect the component types
    for (const auto& pair : component_map)
        component_types.push_back(pair.first);

    return component_types; // Return the list of component types
}

bool Registry::RemoveComponent(Entity entity, uint32_t component_runtime_id)
{
    assert(entity.GetID() < entities_.size() && "Entity ID out of range");
    assert(entity_validity_[entity.GetID()] && "Entity is invalid");
    assert(entities_[entity.GetID()] == entity && "Entity generation mismatch");
    assert(
        entity_to_components_[entity].find(component_runtime_id) != entity_to_components_[entity].end() && 
        "Entity does not have the component");

    // Remove the component from the entity's component map
    entity_to_components_[entity].erase(component_runtime_id);

    // Remove the entity from the component to entities map
    if (component_to_entities_.find(component_runtime_id) != component_to_entities_.end())
        component_to_entities_[component_runtime_id].erase(entity);

    return true; // Component removal successful
}

const std::set<Entity>& Registry::View(uint32_t component_runtime_id) const
{
    static const std::set<Entity> empty_set;

    // Check if the component ID exists in the map
    auto it = component_to_entities_.find(component_runtime_id);
    if (it != component_to_entities_.end())
        return it->second; // Return the set of entities having the component

    return empty_set; // Return an empty set if component ID not found
}

bool Registry::CheckEntityExists(Entity entity) const
{
    if (entity.GetID() >= entities_.size())
        return false; // Entity ID out of range

    if (!entity_validity_[entity.GetID()])
        return false; // Entity is invalid

    if (entities_[entity.GetID()] != entity)
        return false; // Entity generation mismatch

    return true; // Entity is valid
}

Actor::Actor(Entity entity, Registry& registry)
    : entity_(entity), registry_(registry)
{
}

bool Actor::IsValid() const
{
    return registry_.CheckEntityValidity(entity_);
}

} // namespace tecs