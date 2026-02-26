#pragma once

namespace tecs::game::platform
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

} // namespace tecs::game::platform