#include "pch.h"
#include "platform/window_container.h"

namespace tecs::game::platform
{

void WindowContainer::AddWindow(tecs::ID id, std::unique_ptr<Window> window)
{
    assert(window != nullptr && "Cannot add a null window to the container");
    assert(windows_.find(id) == windows_.end() && "Window with the same ID already exists in the container");

    // Add the window to the container
    windows_.emplace(id, std::move(window));
}

Window* WindowContainer::GetWindow(tecs::ID id) const
{
    auto it = windows_.find(id);
    if (it != windows_.end())
        return it->second.get();

    return nullptr; // Window not found
}

std::unique_ptr<Window> WindowContainer::RemoveWindow(tecs::ID id)
{
    // Find the window in the container
    auto it = windows_.find(id);

    if (it == windows_.end())
        return nullptr; // Window not found
    
    // Move the unique pointer out of the container and erase the entry
    std::unique_ptr<Window> removed_window = std::move(it->second);

    // Erase the entry from the container
    windows_.erase(it);

    // Return the removed window
    return removed_window;
}

uint32_t WindowContainer::GetWindowCount() const
{
    // Return the number of windows currently in the container
    return static_cast<uint32_t>(windows_.size());
}

} // namespace tecs::game::platform