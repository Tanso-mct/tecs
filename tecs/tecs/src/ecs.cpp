#include "pch.h"
#include "tecs/ecs.h"

namespace tecs
{

// Initialize static member
uint32_t Component::next_id_ = 0;

Entity::Entity()
{
    // Initialize as invalid entity
    bits_ = MakeBits(0, 0, false);
}

Entity::Entity(uint32_t id, uint32_t gen)
{
    // Initialize as valid entity with given id and generation
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

    // Get IDs for comparison
    uint32_t this_id = GetID();
    uint32_t other_id = other.GetID();

    // Compare IDs if generations are equal
    return this_id < other_id;
}

uint64_t Entity::MakeBits(size_t id, size_t gen, bool valid)
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

Component::Config::Config(std::unique_ptr<Reflection> reflection)
    : reflection_(std::move(reflection))
{
    assert(reflection_ != nullptr && "Reflection object cannot be null");
}

const Reflection& Component::Config::GetReflection() const
{
    assert(reflection_ != nullptr && "Reflection object is null");
    return *reflection_;
}

Component::Component(std::string_view name, const GUID& guid)
    : name_(name), guid_(guid)
{

}

Entity World::CreateEntity()
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

bool World::CommitEntity(Entity entity)
{
    assert(CheckEntityCondition(entity) && "Entity is invalid or does not meet conditions");

    // Add the entity to the component to entities map for each component it has
    for (const auto& pair : entities_to_components_map_[entity])
    {
        uint32_t component_id = pair.first;
        component_to_entities_map_[component_id].insert(entity);
    }

    return true; // Commit successful
}

bool World::DestroyEntity(Entity entity)
{
    assert(CheckEntityCondition(entity) && "Entity is invalid or does not meet conditions");

    // Get component ids which the entity has
    std::vector<uint32_t> component_ids = GetHavingComponents(entity);

    // For each component id, remove the entity from the component to entities map
    for (const auto& component_id : component_ids)
        RemoveComponent(entity, component_id);

    // Mark the entity as invalid
    entity_validity_[entity.GetID()] = false;

    // Add the entity to the free entities list for reuse
    free_entities_.push_back(entity);

    return true; // Destroy successful
}

bool World::CheckEntityValidity(Entity entity) const
{
    assert(entity.GetID() < entities_.size() && "Entity ID out of range");
    return entity_validity_[entity.GetID()] && (entities_[entity.GetID()] == entity);
}

bool World::AddComponent(Entity entity, uint32_t component_id, std::unique_ptr<Component> component)
{
    assert(CheckEntityCondition(entity) && "Entity is invalid or does not meet conditions");
    
    assert(component != nullptr && "Component pointer cannot be null");
    assert(
        entities_to_components_map_[entity].find(component_id) == entities_to_components_map_[entity].end() && 
        "Entity already has the component");

    // Add the component to the entity's component map
    entities_to_components_map_[entity][component_id] = std::move(component);

    // Add the entity to the component to entities map if committed
    if (component_to_entities_map_.find(component_id) != component_to_entities_map_.end())
        component_to_entities_map_[component_id].insert(entity);

    return true; // Component addition successful
}

bool World::RemoveComponent(Entity entity, uint32_t component_id)
{
    assert(entity.GetID() < entities_.size() && "Entity ID out of range");
    assert(entity_validity_[entity.GetID()] && "Entity is invalid");
    assert(entities_[entity.GetID()] == entity && "Entity generation mismatch");
    assert(
        entities_to_components_map_[entity].find(component_id) != entities_to_components_map_[entity].end() && 
        "Entity does not have the component");

    // Remove the component from the entity's component map
    entities_to_components_map_[entity].erase(component_id);

    // Remove the entity from the component to entities map
    if (component_to_entities_map_.find(component_id) != component_to_entities_map_.end())
        component_to_entities_map_[component_id].erase(entity);

    return true; // Component removal successful
}

bool World::HasComponent(Entity entity, uint32_t component_id) const
{
    assert(CheckEntityCondition(entity) && "Entity is invalid or does not meet conditions");

    // Check if the entity exists in the entities to components map
    auto entity_it = entities_to_components_map_.find(entity);
    if (entity_it == entities_to_components_map_.end())
        return false; // Entity not found

    // Get the component map for the entity
    const auto& component_map = entity_it->second;

    // Check if the component exists in the entity's component map
    return component_map.find(component_id) != component_map.end();
}

Component* World::GetComponent(Entity entity, uint32_t component_id)
{
    assert(CheckEntityCondition(entity) && "Entity is invalid or does not meet conditions");

    // Check if the entity exists in the entities to components map
    auto entity_it = entities_to_components_map_.find(entity);
    if (entity_it == entities_to_components_map_.end())
        return nullptr; // Entity not found

    // Get the component map for the entity
    const auto& component_map = entity_it->second;

    // Find the component in the entity's component map
    auto component_it = component_map.find(component_id);
    if (component_it == component_map.end())
        return nullptr; // Component not found

    return component_it->second.get(); // Return pointer to the component
}

std::vector<uint32_t> World::GetHavingComponents(Entity entity) const
{
    // Get the component map for the entity
    const std::unordered_map<uint32_t, std::unique_ptr<Component>>& component_map = entities_to_components_map_.at(entity);

    // Prepare a vector to hold the component IDs
    std::vector<uint32_t> component_ids;

    // Iterate over the component map and collect the IDs
    for (const auto& pair : component_map)
        component_ids.push_back(pair.first);

    return component_ids; // Return the list of component IDs
}

EntityHandle World::CreateEntityHandle(Entity entity)
{
    return EntityHandle(*this, entity);
}

const std::set<Entity>& World::View(uint32_t component_id) const
{
    static const std::set<Entity> empty_set;

    // Check if the component ID exists in the map
    auto it = component_to_entities_map_.find(component_id);
    if (it != component_to_entities_map_.end())
        return it->second; // Return the set of entities having the component

    return empty_set; // Return an empty set if component ID not found
}

bool World::CheckEntityCondition(Entity entity) const
{
    if (entity.GetID() > entities_.size())
        return false; // Entity ID out of range

    if (!entity_validity_[entity.GetID()])
        return false; // Entity is invalid

    if (entities_[entity.GetID()] != entity)
        return false; // Entity generation mismatch

	return true; // Entity meets all conditions
}

EntityHandle::EntityHandle(World& world, Entity entity) : 
    world_(world), 
    entity_(entity)
{
}

bool EntityHandle::IsValid() const
{
    return world_.CheckEntityValidity(entity_);
}

bool EntityHandle::Commit()
{
    return world_.CommitEntity(entity_);
}

bool EntityHandle::AddComponent(uint32_t component_id, std::unique_ptr<Component> component)
{
    return world_.AddComponent(entity_, component_id, std::move(component));
}

bool EntityHandle::RemoveComponent(uint32_t component_id)
{
    return world_.RemoveComponent(entity_, component_id);
}

bool EntityHandle::HasComponent(uint32_t component_id) const
{
    return world_.HasComponent(entity_, component_id);
}

Component* EntityHandle::GetComponent(uint32_t component_id)
{
    return world_.GetComponent(entity_, component_id);
}

std::vector<uint32_t> EntityHandle::GetHavingComponents() const
{
    return world_.GetHavingComponents(entity_);
}

bool EntityHandle::Destroy()
{
    return world_.DestroyEntity(entity_);
}

EntityObject::EntityObject(EntityHandle entity_handle) :
    entity_handle_(entity_handle)
{
}

bool EntityObject::IsValid() const
{
    return entity_handle_.IsValid();
}

void EntityObject::Destroy()
{
    // Call OnDestroy hook
    OnDestroy();

    // Destroy the associated entity
    entity_handle_.Destroy();
}

void EntityObjectGraph::AddEntityObject(std::unique_ptr<EntityObject> entity_object)
{
    entity_objects_.push_back(std::move(entity_object));
}

bool EntityObjectGraph::Compile()
{
    // Clear previous update order
    update_order_.clear();

    // Prepare a list to hold indices of entity objects going to be removed
    std::vector<size_t> going_to_remove_indices;

    // Iterate over entity objects to collect those going to be removed
    for (size_t i = 0; i < entity_objects_.size(); ++i)
    {
        // Get the entity object
        EntityObject& entity_object = *entity_objects_[i];

        // Check if the entity object is going to be removed
        if (!entity_object.IsValid())
            going_to_remove_indices.push_back(i); // Mark for destruction
    }

    // Remove entity objects marked for destruction
    if (!going_to_remove_indices.empty())
    {
        for (size_t i = going_to_remove_indices.size(); i > 0; --i)
        {
            // Get the index of the entity object to remove
			size_t index = going_to_remove_indices[i - 1];

            // Erase the entity object from the list
            entity_objects_.erase(entity_objects_.begin() + index);
        }
    }

    // Iterate over entity objects to determine update order
    for (size_t i = 0; i < entity_objects_.size(); ++i)
        update_order_.push_back(i); // Simple sequential order for now

    if (update_order_.empty())
        return false; // Compilation failed (no entity objects)

    return true; // Compilation successful
}

bool EntityObjectGraph::Update(float delta_time)
{
    // Update entity objects in the determined order
    for (const auto& index : update_order_)
    {
        EntityObject* entity_object = entity_objects_[index].get();
        assert(entity_object != nullptr && "EntityObject should not be null");

        if (!entity_object->IsStarted())
        {
            // Call OnStart
            if (!entity_object->OnStart())
                return false; // Stop update if OnStart fails

            // Mark as started
            entity_object->MarkStarted();
        }
        else
        {
            // Update the entity object
            if (!entity_object->OnUpdate(delta_time))
                return false; // Stop update if any entity object update fails
        }
    }

    return true; // Update successful
}

EntityObjectSpawner::EntityObjectSpawner(World& world, EntityObjectGraph& entity_object_graph) :
    world_(world),
    entity_object_graph_(entity_object_graph)
{
}

bool System::Update(World& world, EntityObjectGraph& entity_object_graph)
{
    if (is_first_update_)
    {
        // Initialize last update time on first update
        last_update_time_ = std::chrono::steady_clock::now();
        is_first_update_ = false;
    }

    // Calculate delta time since last update
    auto current_time = std::chrono::steady_clock::now();
    float delta_time = (current_time - last_update_time_).count();

    // Update last update time
    last_update_time_ = current_time;

    // Loop control flag
    bool loop_continue = true;

    // Compile the entity object graph
    loop_continue = entity_object_graph.Compile();
    if (!loop_continue)
        return false; // Stop update if compilation fails

    // Update the entity object graph
    loop_continue = entity_object_graph.Update(delta_time);
    if (!loop_continue)
        return false; // Stop update if update stops

    // Iterate over all component IDs
    for (uint32_t component_id = 0; component_id < Component::GetMaxID(); component_id++)
    {
        // Iterate over all entities having the component
        for (Entity entity : world.View(component_id))
        {
            // Get the component
            Component* component = world.GetComponent(entity, component_id);
            
            // Create entity handle
            EntityHandle entity_handle = world.CreateEntityHandle(entity);

            // Update the component
            component->Update(entity_handle, delta_time);
        }
    }

    return true; // Update successful
}

} // namespace tecs