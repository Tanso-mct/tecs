#pragma once

// TECS
#include "tecs/file_io.h"
#include "tecs/job.h"
#include "tecs/system.h"
#include "tecs/registry.h"
#include "tecs/debug.h"

namespace tecs
{

// Support structure to hold the main application state for TECS
class TECSContext
{
public:
    TECSContext() : 
        job_scheduler_(), 
        job_tracker_(job_scheduler_.GetWorkerThreads()), 
        debug_(job_scheduler_), 
        debug_log_(debug_) 
    {
    }

    JobScheduler job_scheduler_;
    JobTracker job_tracker_;

    Debug debug_;
    DebugLog debug_log_;

    Registry registry_;

    std::vector<std::unique_ptr<System>> systems_;
    SystemViewList system_view_list_;
};

// Define log exporter IDs for different types of log exporters
enum class LOG_EXPORTER_ID : uint32_t
{
    JOB_LOG_EXPORTER = 1,
    SYSTEM_LOG_EXPORTER = 2,
    ENTITY_LOG_EXPORTER = 3
};

// Support function to initialize the debug functionality and create log exporters
inline bool InitializeDebug(TECSContext& tecs_context)
{
    // Create job log exporter
    std::unique_ptr<LogExporter> job_log_exporter = std::make_unique<JobLogExporter>(tecs_context.job_tracker_);

    // Create system log exporter with system views
    std::vector<std::unique_ptr<SystemView>> system_views;
    for (int component_runtime_id = 0; component_runtime_id < Component::GetMaxRuntimeID(); ++component_runtime_id)
    {
        std::unique_ptr<SystemView> system_view = tecs_context.system_view_list_.GetView(component_runtime_id);
        if (system_view)
            system_views.push_back(std::move(system_view));
    }
    std::unique_ptr<LogExporter> system_log_exporter = std::make_unique<SystemLogExporter>(std::move(system_views));

    // Create entity log exporter
    std::unique_ptr<LogExporter> entity_log_exporter = std::make_unique<EntityLogExporter>(tecs_context.registry_);
    
    // Add log exporters to the Debug instance
    tecs_context.debug_.AddLogExporter((uint32_t)LOG_EXPORTER_ID::JOB_LOG_EXPORTER, std::move(job_log_exporter));
    tecs_context.debug_.AddLogExporter((uint32_t)LOG_EXPORTER_ID::SYSTEM_LOG_EXPORTER, std::move(system_log_exporter));
    tecs_context.debug_.AddLogExporter((uint32_t)LOG_EXPORTER_ID::ENTITY_LOG_EXPORTER, std::move(entity_log_exporter));

    return true;
};

// Support function to update the registry by iterating through all components and updating them
inline bool UpdateRegistry(Registry& registry)
{
    // Get max component runtime ID
    uint32_t max_runtime_id = Component::GetMaxRuntimeID();

    if (max_runtime_id == 0)
        return false; // No components to update

    // Iterate through all component runtime IDs and update the registry
    for (uint32_t component_runtime_id = 0; component_runtime_id < max_runtime_id; ++component_runtime_id)
    {
        for (const Entity& entity : registry.View(component_runtime_id))
        {
            // Get the component for the entity and component type
            Component* component = registry.GetComponent(entity, component_runtime_id);
            if (component)
            {
                // Create an actor for the entity
                Actor actor(entity, registry);

                // Update the component with the actor
                if (!component->Update(actor))
                    return false; // Component update failed
            }
        }
    }
    
    return true;
}

// Support function to update all systems by flushing their task queues
inline bool UpdateSystems(std::vector<std::unique_ptr<System>>& systems)
{
    for (auto& system : systems)
    {
        if (!system->FlushTasks())
            return false; // System task flush failed
    }
    
    return true;
}

} // namespace tecs