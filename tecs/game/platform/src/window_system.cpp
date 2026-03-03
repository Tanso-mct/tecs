#include "pch.h"
#include "platform/window_system.h"

#include "platform/window_context.h"

namespace tecs::game::platform
{

WindowSystem::WindowSystem(JobScheduler& job_scheduler) :
    System(job_scheduler, std::make_unique<WindowContext>())
{
}

} // namespace tecs::game::platform