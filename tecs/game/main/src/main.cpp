#include <iostream>

#include "tecs/tecs.h"

namespace tecs_game_main
{

// Function to initialize systems and create system views
bool InitializeSystems(tecs::TECSContext& tecs_context)
{
    // TODO: Implement system creation and store views in to tecs_context.system_view_list_

    return true;
}

// Function to initialize entities in the registry
bool InitializeEntity(tecs::TECSContext& tecs_context)
{
    // TODO: Implement initial entity creation and component assignment in the registry

    return true;
}

} // namespace tecs_game_main

int main(int argc, char* argv[])
{
    // Create the tecs context
    tecs::TECSContext tecs_context;

    // Initialize systems and create system views
    bool systems_initialized = tecs_game_main::InitializeSystems(tecs_context);
    if (!systems_initialized)
    {
        std::cerr << "Failed to initialize systems." << std::endl;
        return 1;
    }

    // Initialize entities in the registry
    bool entities_initialized = tecs_game_main::InitializeEntity(tecs_context);
    if (!entities_initialized)
    {
        std::cerr << "Failed to initialize entities." << std::endl;
        return 1;
    }

    // Initialize tecs's debug functionality after systems and entities are set up
    bool debug_initialized = tecs::InitializeDebug(tecs_context);
    if (!debug_initialized)
    {
        std::cerr << "Failed to initialize debug." << std::endl;
        return 1;
    }

    // Main loop
    while (true)
    {
        if (!tecs::UpdateRegistry(tecs_context.registry_))
            break; // Exit loop if registry update fails

        if (!tecs::UpdateSystems(tecs_context.systems_))
            break; // Exit loop if system update fails
    }
    
    return 0;
}   