#pragma once

// TECS
#include "tecs/tecs.h"

// Platform
#include "platform/window_container.h"

namespace tecs::game::platform
{

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

    /**
     * @brief
     * Get a reference to the WindowContainer
     * 
     * @return WindowContainer& 
     * Reference to the WindowContainer
     */
    WindowContainer& GetWindowContainer() { return window_container_; }

    /**
     * @brief
     * Get a const reference to the WindowContainer
     * 
     * @return const WindowContainer&
     * Const reference to the WindowContainer
     */
    const WindowContainer& GetWindowContainer() const { return window_container_; }

private:
    // Container for managing windows
    WindowContainer window_container_;

};

} // namespace tecs::game::platform