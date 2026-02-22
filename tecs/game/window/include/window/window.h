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
    
class WindowContext :
    public Service::Context
{
public:
    WindowContext() = default;
    ~WindowContext() override = default;

private:
    

};

class WindowService :
    public Service
{
public:

};

} // tecs::game::window
