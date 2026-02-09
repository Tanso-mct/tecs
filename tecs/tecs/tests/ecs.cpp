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
    TransformComponent() :
        ComponentBase<TransformComponent>("Transform", GUID())
    {
    }

    ~TransformComponent() = default;

    // --- Component interface implementation ---

    class TransformConfig : public Config
    {
    public:
        TransformConfig() : 
            Config(std::make_unique<Reflection>())
        {

        }

        float x = 0.0f;
        float y = 0.0f;
    };

    bool Import(std::unique_ptr<Config> config) override
    {
        // Cast the config to TransformConfig
        TransformConfig* transform_config = dynamic_cast<TransformConfig*>(config.get());
        if (!transform_config)
        {
            std::cerr << "Invalid config type for TransformComponent import." << std::endl;
            return false; // Invalid config type
        }

        // Import position data
        x_ = transform_config->x;
        y_ = transform_config->y;

        return true; // Import successful
    }

    std::unique_ptr<Config> Export() const override
    {
        // Create a new TransformConfig
        std::unique_ptr<TransformConfig> config = std::make_unique<TransformConfig>();

        // Export position data
        config->x = x_;
        config->y = y_;

        return config; // Return the exported config
    }

    void Update(float delta_time) override
    {
        std::cout << "Updating TransformComponent with delta_time: " << delta_time << std::endl;
    }

    // --- Getters and Setters for position ---

    float GetX() const { return x_; }
    void SetX(float new_x) { x_ = new_x; }

    float GetY() const { return y_; }
    void SetY(float new_y) { y_ = new_y; }

private:
    float x_ = 0.0f;
    float y_ = 0.0f;
};

/**
 * @brief
 * Sample component representing a Velocity
 */
class VelocityComponent : public ComponentBase<VelocityComponent>
{
public:
    VelocityComponent() :
        ComponentBase<VelocityComponent>("Velocity", GUID())
    {
    }

    ~VelocityComponent() = default;

    // --- Component interface implementation ---

    class VelocityConfig : public Config
    {
    public:
        VelocityConfig() : 
            Config(std::make_unique<Reflection>())
        {

        }

        float vx = 0.0f;
        float vy = 0.0f;
    };

    bool Import(std::unique_ptr<Config> config) override
    {
        // Cast the config to VelocityConfig
        VelocityConfig* velocity_config = dynamic_cast<VelocityConfig*>(config.get());
        if (!velocity_config)
        {
            std::cerr << "Invalid config type for VelocityComponent import." << std::endl;
            return false; // Invalid config type
        }

        // Import velocity data
        vx_ = velocity_config->vx;
        vy_ = velocity_config->vy;

        return true; // Import successful
    }

    std::unique_ptr<Config> Export() const override
    {
        // Create a new VelocityConfig
        std::unique_ptr<VelocityConfig> config = std::make_unique<VelocityConfig>();

        // Export velocity data
        config->vx = vx_;
        config->vy = vy_;

        return config; // Return the exported config
    }

    void Update(float delta_time) override
    {
        std::cout << "Updating VelocityComponent with delta_time: " << delta_time << std::endl;
    }

    // --- Getters and Setters for velocity ---

    float GetVX() const { return vx_; }
    void SetVX(float new_vx) { vx_ = new_vx; }

    float GetVY() const { return vy_; }
    void SetVY(float new_vy) { vy_ = new_vy; }

private:
    float vx_ = 0.0f;
    float vy_ = 0.0f;
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

TEST(tecs, entity_unpack)
{
    constexpr uint32_t kTestId = 5;
    constexpr uint32_t kTestGen = 10;

    // Create an entity with ID 1 and generation 0
    tecs::Entity entity(kTestId, kTestGen);

    // Verify the entity's ID and generation
    EXPECT_EQ(entity.GetID(), kTestId);
    EXPECT_EQ(entity.GetGen(), kTestGen);
    EXPECT_TRUE(entity.IsValid());
}

TEST(tecs, entity_comparison)
{
    // Create entities for comparison
    tecs::Entity entity1(1, 0);
    tecs::Entity entity2(1, 0);
    tecs::Entity entity3(2, 0);
    tecs::Entity entity4(1, 1);

    // Test equality and inequality operators
    EXPECT_TRUE(entity1 == entity2);
    EXPECT_FALSE(entity1 != entity2);
    EXPECT_TRUE(entity1 != entity3);
    EXPECT_TRUE(entity1 != entity4);

    // Test less-than operator
    EXPECT_TRUE(entity1 < entity3); // ID comparison
    EXPECT_TRUE(entity1 < entity4); // Generation comparison
    EXPECT_FALSE(entity3 < entity1);
    EXPECT_FALSE(entity4 < entity1);
}

TEST(tecs, component_creation)
{
    // --- Transform ---

    // Test data for position
    constexpr float kTestX = 15.0f;
    constexpr float kTestY = 25.0f;

    // Create instance of transform component
    std::unique_ptr<tecs::test::TransformComponent> transform_component 
        = std::make_unique<tecs::test::TransformComponent>();
    {
        // Create a config for the transform component
        std::unique_ptr<tecs::test::TransformComponent::TransformConfig> config 
            = std::make_unique<tecs::test::TransformComponent::TransformConfig>();

        // Set position values in the config
        config->x = kTestX;
        config->y = kTestY;

        // Import the config into the component
        bool import_result = transform_component->Import(std::move(config));
        EXPECT_TRUE(import_result);
    }

    // Verify that the component has the correct position values
    EXPECT_FLOAT_EQ(transform_component->GetX(), kTestX);
    EXPECT_FLOAT_EQ(transform_component->GetY(), kTestY);

    // --- Velocity ---

    // Test data for velocity
    constexpr float kTestVX = 5.0f;
    constexpr float kTestVY = 10.0f;

    // Create instance of velocity component
    std::unique_ptr<tecs::test::VelocityComponent> velocity_component
        = std::make_unique<tecs::test::VelocityComponent>();
    {
        // Create a config for the velocity component
        std::unique_ptr<tecs::test::VelocityComponent::VelocityConfig> config 
            = std::make_unique<tecs::test::VelocityComponent::VelocityConfig>();

        // Set velocity values in the config
        config->vx = kTestVX;
        config->vy = kTestVY;

        // Import the config into the component
        bool import_result = velocity_component->Import(std::move(config));
        EXPECT_TRUE(import_result);
    }

    // Verify that the component has the correct velocity values
    EXPECT_FLOAT_EQ(velocity_component->GetVX(), kTestVX);
    EXPECT_FLOAT_EQ(velocity_component->GetVY(), kTestVY);

    // --- Update ---

    constexpr float kDeltaTime = 0.16f; // Example delta time

    transform_component->Update(kDeltaTime);
    velocity_component->Update(kDeltaTime);
}