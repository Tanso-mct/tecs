#pragma once

// TECS
#include "tecs/tecs.h"

// STL
#include <unordered_map>
#include <memory>

// Platform
#include "platform/window.h"

namespace tecs::game::platform
{

class WindowContainer
{
public:
    WindowContainer() = default;
    ~WindowContainer() = default;

    /**
     * @brief
     * Add a window to the container
     * 
     * @param id 
     * The ID of the window to add
     * 
     * @param window 
     * The unique pointer to the window to add
     */
    void AddWindow(tecs::ID id, std::unique_ptr<Window> window);

    /**
     * @brief
     * Get a pointer to the window with the given ID
     * 
     * @param id 
     * The ID of the window to get
     * 
     * @return Window* 
     * A pointer to the window if found, nullptr otherwise
     */
    Window* GetWindow(tecs::ID id) const;

    /**
     * @brief
     * Remove the window with the given ID from the container
     * 
     * @param id 
     * The ID of the window to remove
     * 
     * @return std::unique_ptr<Window> 
     * The unique pointer to the removed window if found, nullptr otherwise
     */
    std::unique_ptr<Window> RemoveWindow(tecs::ID id);

    /**
     * @brief
     * Get the number of windows currently in the container
     * 
     * @return uint32_t 
     * The number of windows in the container
     */
    uint32_t GetWindowCount() const;

private:
    // Map of window IDs to their corresponding windows
    std::unordered_map<tecs::ID, std::unique_ptr<Window>> windows_;
};

} // namespace tecs::game::platform