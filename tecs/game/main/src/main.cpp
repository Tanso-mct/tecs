#include <iostream>

#include "tecs/tecs.h"

namespace tecs_game_main
{

// Function to initialize systems and create system views
std::vector<std::unique_ptr<tecs::SystemView>> InitializeSystems(
    tecs::JobScheduler& job_scheduler, 
    tecs::Registry& registry, 
    tecs::SystemViewList& system_view_list)
{
    return std::vector<std::unique_ptr<tecs::SystemView>>();
}

// Define log exporter IDs for different types of log exporters
enum class LOG_EXPORTER_ID : uint32_t
{
    JOB_LOG_EXPORTER = 1,
    SYSTEM_LOG_EXPORTER = 2,
    ENTITY_LOG_EXPORTER = 3
};

// Function to initialize the debug system and create log exporters
bool InitializeDebug(
    tecs::JobScheduler& job_scheduler, tecs::JobTracker& job_tracker,tecs::Registry& registry, 
    tecs::SystemViewList& system_view_list, std::vector<std::unique_ptr<tecs::SystemView>> system_views)
{
    // Create a Debug instance for managing log exporters and exporting logs
    std::unique_ptr<tecs::Debug> debug = std::make_unique<tecs::Debug>(job_scheduler);

    // Create a DebugLog instance for exporting logs using the Debug instance
    std::unique_ptr<tecs::DebugLog> debug_log = std::make_unique<tecs::DebugLog>(*debug);

    // Create exporters
    std::unique_ptr<tecs::LogExporter> job_log_exporter = std::make_unique<tecs::JobLogExporter>(job_tracker);
    std::unique_ptr<tecs::LogExporter> system_log_exporter = std::make_unique<tecs::SystemLogExporter>(std::move(system_views));
    std::unique_ptr<tecs::LogExporter> entity_log_exporter = std::make_unique<tecs::EntityLogExporter>(registry);
    
    // Add log exporters to the Debug instance
    debug->AddLogExporter((uint32_t)LOG_EXPORTER_ID::JOB_LOG_EXPORTER, std::move(job_log_exporter));
    debug->AddLogExporter((uint32_t)LOG_EXPORTER_ID::SYSTEM_LOG_EXPORTER, std::move(system_log_exporter));
    debug->AddLogExporter((uint32_t)LOG_EXPORTER_ID::ENTITY_LOG_EXPORTER, std::move(entity_log_exporter));
};


} // namespace tecs_game_main

int main(int argc, char* argv[])
{
    // Create job scheduler and job tracker for managing jobs and worker threads
    tecs::JobScheduler job_scheduler;
    tecs::JobTracker job_tracker(job_scheduler.GetWorkerThreads());

    // Create registry for managing entities and components
    tecs::Registry registry;

    // Create system view list for managing system views
    tecs::SystemViewList system_view_list;

    // Initialize systems and create system views
    std::vector<std::unique_ptr<tecs::SystemView>> system_views 
        = tecs_game_main::InitializeSystems(job_scheduler, registry, system_view_list);
    if (system_views.empty())
    {
        std::cerr << "Failed to initialize systems." << std::endl;
        return 1;
    }

    // Initialize debug and create log exporters
    bool debug_initialized 
        = tecs_game_main::InitializeDebug(job_scheduler, job_tracker, registry, system_view_list, std::move(system_views));
    if (!debug_initialized)
    {
        std::cerr << "Failed to initialize debug." << std::endl;
        return 1;
    }
    
    return 0;
}