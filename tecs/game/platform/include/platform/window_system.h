#pragma once

// TECS
#include "tecs/tecs.h"

namespace tecs::game::platform
{

/**
 * @brief
 * System for managing the window and its context
 */
class WindowSystem :
    public System
{
public:
    /**
     * @brief
     * Construct a new Window System object
     * It create window context and initializes the system
     * 
     * @param job_scheduler 
     * Reference to the JobScheduler
     */
    WindowSystem(JobScheduler& job_scheduler);

    /**
     * @brief
     * Destroy the Window System object.
     */
    ~WindowSystem() override;

    // --- System interface implementation ---

    bool PreUpdate() override;
    bool Update() override;
    bool PostUpdate() override;
};

} // namespace tecs::game::platform