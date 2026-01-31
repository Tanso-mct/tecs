#include "pch.h"
#include "tecs/ecs.h"

namespace tecs
{

// Initialize static member
uint32_t Component::next_id_ = 0;
uint32_t System::next_id_ = 0;

} // namespace tecs