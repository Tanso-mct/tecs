#pragma once

// STL
#include <cstdint>
#include <string>

// TECS
#include "tecs/tecs.h"

namespace tecs::game::window
{

/**
 * @brief
 * Interface for the Window
 * You can implement this interface for different platforms (e.g., Win32, X11, etc.)
 */
class Window
{
public:
    virtual ~Window() = default;
};
    
/**
 * @brief
 * Context for the WindowSystem
 */
class WindowContext :
    public System::Context
{
public:
    WindowContext() = default;
    ~WindowContext() override = default;

private:
    

};

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

} // tecs::game::window
