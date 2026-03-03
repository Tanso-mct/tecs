#include "pch.h"
#include "tecs/ecs.h"

namespace tecs
{

// Initialize static member
uint32_t Component::next_id_ = 0;
uint32_t System::next_id_ = 0;

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

std::unique_ptr<EntityHandle> EntityHandle::Clone() const
{
    return std::make_unique<EntityHandle>(world_, entity_);
}

System::Task::Task(Func func) : 
    func_(func)
{
    assert(func_ && "Task function cannot be nullptr");

    // Create default Info if none provided
    info_ = std::make_unique<Info>();
}

System::Task::Task(Func func, std::unique_ptr<Info> info) :
    func_(func), 
    info_(std::move(info))
{
    assert(func_ && "Task function cannot be nullptr");
    assert(info_ && "Task Info cannot be nullptr");
}

bool System::Task::Execute(Context& context, JobScheduler& job_scheduler)
{
    return func_(context, job_scheduler);
}

const System::Task::Info& System::Task::GetInfo() const
{
    assert(info_ && "Task Info is nullptr");
    return *info_;
}

System::System(JobScheduler& job_scheduler, std::unique_ptr<Context> context) :
    job_scheduler_(job_scheduler), 
    context_(std::move(context))
{
    assert(context_ && "System Context cannot be nullptr");
}

void System::SumbitTaskList(TaskList tasks)
{
    // Enqueue the provided task list
    task_list_queue_.Enqueue(std::move(tasks));
}

const System::Context& System::GetContext() const
{
    assert(context_ && "System Context is nullptr");
    return *context_;
}

void System::TaskListQueue::Enqueue(TaskList tasks)
{
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.emplace(std::move(tasks));
}

bool System::TaskListQueue::Dequeue(TaskList& tasks)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty())
        return false; // If the queue is empty, return false

    // Move the front task list to the provided reference
    tasks = std::move(queue_.front());
    queue_.pop();

    return true; // Successfully dequeued a task list
}

std::vector<System::TaskList> System::TaskListQueue::Dequeue()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<TaskList> all_tasks;

    while (!queue_.empty())
    {
        // Move each task list to the vector
        all_tasks.emplace_back(std::move(queue_.front()));
        queue_.pop();
    }

    return all_tasks; // Return all dequeued task lists
}

SystemProxy::SystemProxy(System& system) : 
    system_(system)
{
}

void SystemProxy::SumbitTaskList(System::TaskList tasks)
{
    system_.SumbitTaskList(std::move(tasks));
}

std::unique_ptr<SystemProxy> SystemProxy::Clone() const
{
    return std::make_unique<SystemProxy>(system_);
}

void SystemProxyManager::RegisterSystemProxy(uint32_t system_id, std::unique_ptr<SystemProxy> proxy)
{
    // Lock the mutex for thread-safe access
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = system_proxies_.find(system_id);
    if (it != system_proxies_.end())
    {
        // If a proxy for this system ID already exists, replace it
        it->second = std::move(proxy);
    }
    else
    {
        // Otherwise, insert the new proxy
        system_proxies_.emplace(system_id, std::move(proxy));
    }
}

std::unique_ptr<SystemProxy> SystemProxyManager::GetSystemProxy(uint32_t system_id)
{
    // Lock the mutex for thread-safe access
    std::lock_guard<std::mutex> lock(mutex_);

    // Find the SystemProxy for the specified system ID
    auto it = system_proxies_.find(system_id);
    assert(it != system_proxies_.end() && "SystemProxy for the specified system ID not found");

    // Return a clone of the SystemProxy
    assert(it->second && "SystemProxy for the specified system ID is nullptr");
    return it->second->Clone(); // Return a clone of the SystemProxy
}

} // namespace tecs