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

private:
    // Container for managing windows
    WindowContainer window_container_;

};

} // namespace tecs::game::platform