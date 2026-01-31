// Google test
#include <gtest/gtest.h>

// TECS
#include "tecs/tecs.h"

namespace tecs::test
{

/**
 * @brief
 * Sample component representing a Transform
 */
class TransformComponent : public ComponentBase<TransformComponent>
{
public:
    float x = 0.0f;
    float y = 0.0f;
};

/**
 * @brief
 * Sample component representing a Velocity
 */
class VelocityComponent : public ComponentBase<VelocityComponent>
{
public:
    float vx = 0.0f;
    float vy = 0.0f;
};

} // namespace tecs::test

TEST(tecs, component_id)
{
    // Get unique IDs for the sample components
    uint32_t transform_id = tecs::test::TransformComponent::ID();
    uint32_t velocity_id = tecs::test::VelocityComponent::ID();

    // Ensure the IDs are unique
    EXPECT_NE(transform_id, velocity_id);
}