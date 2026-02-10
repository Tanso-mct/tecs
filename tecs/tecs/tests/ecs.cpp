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

    void Update(EntityHandle entity_handle, float delta_time) override
    {
        std::cout << "Updating TransformComponent with delta_time: " << delta_time << std::endl;
        std::cout << "Current Position: (" << x_ << ", " << y_ << ")" << std::endl;
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

    void Update(EntityHandle entity_handle, float delta_time) override
    {
        std::cout << "Updating VelocityComponent with delta_time: " << delta_time << std::endl;
        std::cout << "Current Velocity: (" << vx_ << ", " << vy_ << ")" << std::endl;

        // Get the TransformComponent
        TransformComponent* transform = entity_handle.GetComponent<TransformComponent>();
        if (!transform)
            std::cerr << "Entity does not have a TransformComponent." << std::endl;

        // Update position based on velocity
        float new_x = transform->GetX() + vx_ * delta_time;
        float new_y = transform->GetY() + vy_ * delta_time;
        transform->SetX(new_x);
        transform->SetY(new_y);
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

class SampleObject : public EntityObject
{
public:
    SampleObject(EntityHandle entity_handle) :
        EntityObject(entity_handle)
    {
        // Add transform component
        std::unique_ptr<TransformComponent::TransformConfig> transform_config 
            = std::make_unique<TransformComponent::TransformConfig>();
        transform_config->x = 0.0f;
        transform_config->y = 0.0f;
        entity_handle.AddComponent<TransformComponent>(std::move(transform_config));

        // Add velocity component
        std::unique_ptr<VelocityComponent::VelocityConfig> velocity_config
            = std::make_unique<VelocityComponent::VelocityConfig>();
        velocity_config->vx = 1.0f;
        velocity_config->vy = 1.0f;
        entity_handle.AddComponent<VelocityComponent>(std::move(velocity_config));
    }

    ~SampleObject() override = default;

    bool Update(float delta_time) override
    {
        // Get command from user
        std::cout << "Enter command (type 'help' for options): ";
        std::string command;
        std::cin >> command;

        if (command == "help")
        {
            std::cout << "Available commands:\n";
            std::cout << "  help       - Show this help message\n";
            std::cout << "  status     - Show current position and velocity\n";
            std::cout << "  setpos x y - Set position to (x, y)\n";
            std::cout << "  setvel vx vy - Set velocity to (vx, vy)\n";
            std::cout << "  destroy    - Destroy this entity object\n";
            std::cout << "  exit       - Exit the update loop\n";
        }
        else if (command == "status")
        {
        }
        else if (command == "setpos")
        {
        }
        else if (command == "setvel")
        {
        }
        else if (command == "destroy")
        {
            std::cout << "Destroying entity object...\n";
            Destroy();
        }
        else if (command == "exit")
        {
            std::cout << "Exiting...\n";
            return false; // Stop updating
        }
        else
        {
            std::cout << "Unknown command. Type 'help' for options.\n";
        }

        return true; // Continue updating
    }
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
}

TEST(tecs, ecs_world)
{
    // Verify const values
    constexpr float kPosX = 100.0f;
    constexpr float kPosY = 200.0f;

    // Create a world instance
    tecs::World world;

    // Create an entity
    tecs::Entity entity = world.CreateEntity();

    // check entity is valid
    EXPECT_TRUE(world.CheckEntityValidity(entity));

    // Create transform component
    std::unique_ptr<tecs::test::TransformComponent> transform_component 
        = std::make_unique<tecs::test::TransformComponent>();

    // Configure transform component
    std::unique_ptr<tecs::test::TransformComponent::TransformConfig> transform_config 
        = std::make_unique<tecs::test::TransformComponent::TransformConfig>();
    transform_config->x = kPosX;
    transform_config->y = kPosY;
    bool import_transform_result = transform_component->Import(std::move(transform_config));
    EXPECT_TRUE(import_transform_result);

    // Add transform component to entity
    bool add_transform_result 
        = world.AddComponent(entity, tecs::test::TransformComponent::ID(), std::move(transform_component));
    EXPECT_TRUE(add_transform_result);

    // Check entity has the transform component
    bool has_transform = world.HasComponent(entity, tecs::test::TransformComponent::ID());
    EXPECT_TRUE(has_transform);

    // Get the transform component from the world
    tecs::Component* retrieved_component 
        = world.GetComponent(entity, tecs::test::TransformComponent::ID());
    EXPECT_NE(retrieved_component, nullptr);

    // Cast to TransformComponent
    tecs::test::TransformComponent* retrieved_transform_component
        = dynamic_cast<tecs::test::TransformComponent*>(retrieved_component);
    EXPECT_NE(retrieved_transform_component, nullptr);

    // Verify the position values
    EXPECT_FLOAT_EQ(retrieved_transform_component->GetX(), kPosX);
    EXPECT_FLOAT_EQ(retrieved_transform_component->GetY(), kPosY);

    // Destroy the entity
    bool destroy_result = world.DestroyEntity(entity);
    EXPECT_TRUE(destroy_result);

    // Verify the entity is no longer valid
    EXPECT_FALSE(world.CheckEntityValidity(entity));
}

TEST(tecs, ecs_world_template)
{
    // Verify const values
    constexpr float kPosX = 100.0f;
    constexpr float kPosY = 200.0f;

    // Create a world instance
    tecs::World world;

    // Create an entity
    tecs::Entity entity = world.CreateEntity();

    // check entity is valid
    EXPECT_TRUE(world.CheckEntityValidity(entity));

    // Create transform component config
    std::unique_ptr<tecs::test::TransformComponent::TransformConfig> transform_config 
        = std::make_unique<tecs::test::TransformComponent::TransformConfig>();
    transform_config->x = kPosX;
    transform_config->y = kPosY;

    // Add transform component to entity using templated method
    bool add_transform_result 
        = world.AddComponent<tecs::test::TransformComponent>(entity, std::move(transform_config));
    EXPECT_TRUE(add_transform_result);

    // Check entity has the transform component
    bool has_transform = world.HasComponent<tecs::test::TransformComponent>(entity);
    EXPECT_TRUE(has_transform);

    // Get the transform component from the world
    tecs::test::TransformComponent* transform_component = world.GetComponent<tecs::test::TransformComponent>(entity);
    EXPECT_NE(transform_component, nullptr);

    // Verify the position values
    EXPECT_FLOAT_EQ(transform_component->GetX(), kPosX);
    EXPECT_FLOAT_EQ(transform_component->GetY(), kPosY);

    // Remove the transform component
    bool remove_transform_result = world.RemoveComponent<tecs::test::TransformComponent>(entity);
    EXPECT_TRUE(remove_transform_result);

    // Destroy the entity
    bool destroy_result = world.DestroyEntity(entity);
    EXPECT_TRUE(destroy_result);

    // Verify the entity is no longer valid
    EXPECT_FALSE(world.CheckEntityValidity(entity));
}

TEST(tecs, system)
{
    // Create a world instance
    tecs::World world;

    // Create an entity
    tecs::Entity entity = world.CreateEntity();

    // Create and add transform component
    std::unique_ptr<tecs::test::TransformComponent::TransformConfig> transform_config 
        = std::make_unique<tecs::test::TransformComponent::TransformConfig>();
    transform_config->x = 0.0f;
    transform_config->y = 0.0f;
    world.AddComponent<tecs::test::TransformComponent>(entity, std::move(transform_config));

    // Create and add velocity component
    std::unique_ptr<tecs::test::VelocityComponent::VelocityConfig> velocity_config 
        = std::make_unique<tecs::test::VelocityComponent::VelocityConfig>();
    velocity_config->vx = 1.0f;
    velocity_config->vy = 1.0f;
    world.AddComponent<tecs::test::VelocityComponent>(entity, std::move(velocity_config));

    // Commit the entity to the world
    // If not committed, the system should not process it
    world.CommitEntity(entity);

    // Create a system that processes all components
    tecs::System system;

    // Create entity object graph
    tecs::EntityObjectGraph entity_object_graph;

    // Update the system
    system.Update(world, entity_object_graph);
}

TEST(tecs, system_with_not_committed_entity)
{
    // Create a world instance
    tecs::World world;

    // Create an entity
    tecs::Entity entity = world.CreateEntity();

    // Create and add transform component
    std::unique_ptr<tecs::test::TransformComponent::TransformConfig> transform_config 
        = std::make_unique<tecs::test::TransformComponent::TransformConfig>();
    transform_config->x = 0.0f;
    transform_config->y = 0.0f;
    world.AddComponent<tecs::test::TransformComponent>(entity, std::move(transform_config));

    // Create and add velocity component
    std::unique_ptr<tecs::test::VelocityComponent::VelocityConfig> velocity_config 
        = std::make_unique<tecs::test::VelocityComponent::VelocityConfig>();
    velocity_config->vx = 1.0f;
    velocity_config->vy = 1.0f;
    world.AddComponent<tecs::test::VelocityComponent>(entity, std::move(velocity_config));

    // Create a system that processes all components
    tecs::System system;

    // Create entity object graph
    tecs::EntityObjectGraph entity_object_graph;

    // Update the system
    system.Update(world, entity_object_graph);
}

TEST(tecs, entity_object)
{
    // Create a world instance
    tecs::World world;

    // Create entity object graph
    tecs::EntityObjectGraph entity_object_graph;

    // Create singleton entity object spawner
    tecs::EntityObjectSpawner spawner(world, entity_object_graph);

    // Create system
    tecs::System system;

    // Spawn a sample entity object
    tecs::EntityHandle entity_handle = spawner.SpawnEntityObject<tecs::test::SampleObject>();

    // Commit the entity to the world
    entity_handle.Commit();

    bool loop = true;
    while (loop)
    {
        // Update the entity object graph
        loop = system.Update(world, entity_object_graph);
    }
}