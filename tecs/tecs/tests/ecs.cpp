// Google test
#include <gtest/gtest.h>

// TECS
#include "tecs/tecs.h"

namespace tecs::test
{

/**
 * @brief
 * Sample implementation of System::Context for testing purposes
 */
class SampleContext : public System::Context
{
public:
    SampleContext() = default;
    ~SampleContext() override = default;

    void ModifyData( tecs::JobScheduler& job_sched,int new_value)
    {
        // Schedule a job to modify the sample data
        tecs::JobHandle job_handle = job_sched.ScheduleJob(tecs::Job([this, new_value]()
        {
            // Modify the sample data
            sample_data_ = new_value;

            // Simulate some work
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }));

        // Wait for the job to complete
        job_handle.Wait();
    }

    int GetData() const { return sample_data_; }

private:
    int sample_data_ = 0;
};

class SampleSystem : public System
{
public:
    SampleSystem(JobScheduler& job_sched) :
        System(job_sched, std::make_unique<SampleContext>())
    {
    }

    ~SampleSystem() override = default;

    bool PreUpdate() override
    {
        // Sample pre-update logic
        return true;
    }

    bool Update() override
    {
        // Dequeue all task lists
        std::vector<TaskList> all_tasks = task_list_queue_.Dequeue();

        // Execute each task in each task list
        for (auto& tasks : all_tasks)
        {
            for (auto& task : tasks)
            {
                if (!task.Execute(*context_, job_scheduler_))
                    return false; // Task execution failed
            }
        }

        return true;
    }

    bool PostUpdate() override
    {
        // Sample post-update logic
        return true;
    }

};

// Constant for modified value in tests
constexpr const int kModifiedValue = 100;

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
            Config(std::make_unique<Reflection>(
                reinterpret_cast<std::byte*>(this), []
                {
                    std::vector<std::unique_ptr<ReflectionField>> fields;
                    fields.push_back(std::make_unique<ReflectionFieldType<float>>("x", offsetof(TransformConfig, x)));
                    fields.push_back(std::make_unique<ReflectionFieldType<float>>("y", offsetof(TransformConfig, y)));
                    return fields;
                }()))
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
            Config(std::make_unique<Reflection>(
                reinterpret_cast<std::byte*>(this), []
                {
                    std::vector<std::unique_ptr<ReflectionField>> fields;
                    fields.push_back(std::make_unique<ReflectionFieldType<float>>("vx", offsetof(VelocityConfig, vx)));
                    fields.push_back(std::make_unique<ReflectionFieldType<float>>("vy", offsetof(VelocityConfig, vy)));
                    return fields;
                }()))
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

class BehaviorComponent : public ComponentBase<BehaviorComponent>
{
public:
    BehaviorComponent() :
        ComponentBase<BehaviorComponent>("Behavior", GUID())
    {
    }

    ~BehaviorComponent() = default;

    // --- Component interface implementation ---

    class BehaviorConfig : public Config
    {
    public:
        BehaviorConfig() : 
            Config(std::make_unique<Reflection>(
                reinterpret_cast<std::byte*>(this), []
                {
                    std::vector<std::unique_ptr<ReflectionField>> fields;
                    fields.push_back(std::make_unique<ReflectionFieldType<bool>>("is_active", offsetof(BehaviorConfig, is_active)));
                    fields.push_back(std::make_unique<ReflectionFieldType<std::string>>("behavior_name", offsetof(BehaviorConfig, behavior_name)));
                    return fields;
                }()))
        {
        }

        bool is_active = true;
        std::string behavior_name;
    };

    bool Import(std::unique_ptr<Config> config) override
    {
        // Cast the config to BehaviorConfig
        BehaviorConfig* behavior_config = dynamic_cast<BehaviorConfig*>(config.get());
        if (!behavior_config)
        {
            std::cerr << "Invalid config type for BehaviorComponent import." << std::endl;
            return false; // Invalid config type
        }

        // Import behavior data
        is_active_ = behavior_config->is_active;
        behavior_name_ = behavior_config->behavior_name;

        return true; // Import successful
    }

    std::unique_ptr<Config> Export() const override
    {
        // Create a new BehaviorConfig
        std::unique_ptr<BehaviorConfig> config = std::make_unique<BehaviorConfig>();

        // Export behavior data
        config->is_active = is_active_;
        config->behavior_name = behavior_name_;

        return config; // Return the exported config
    }

    void Update(EntityHandle entity_handle, float delta_time) override
    {
        std::cout << "Updating BehaviorComponent with delta_time: " << delta_time << std::endl;
        std::cout << "Current Behavior: " << behavior_name_ << std::endl;
        std::cout << "Is Active: " << (is_active_ ? "Yes" : "No") << std::endl;

        // Get system proxy manager
        SystemProxyManager& system_proxy_manager = SystemProxyManager::GetInstance();

        // Create task list
        tecs::System::TaskList task_list;

        // Modify the sample context data in the sample system using a task
        tecs::AddTask<tecs::test::SampleContext>(
            task_list, [this](tecs::test::SampleContext& ctx, tecs::JobScheduler& job_sched) -> bool
        {
            ctx.ModifyData(job_sched, tecs::test::kModifiedValue);
            return true; // Task executed successfully
        });

        // Get system proxy for sample system
        std::unique_ptr<tecs::SystemProxy> sample_proxy =
            system_proxy_manager.GetSystemProxy<tecs::test::SampleSystem>();

        // Submit task list to the system via proxy
        sample_proxy->SumbitTaskList(std::move(task_list));
    }



private:
    // Active state of the behavior
    bool is_active_ = true;

    // Name of the behavior
    std::string behavior_name_;

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

TEST(tecs, ecs_world_template_version)
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

TEST(tecs, entity_handle)
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

    // Create an entity handle for the created entity
    tecs::EntityHandle entity_handle(world, entity);

    // Verify the entity handle is valid
    EXPECT_TRUE(entity_handle.IsValid());

    // Create transform component config
    std::unique_ptr<tecs::test::TransformComponent::TransformConfig> transform_config 
        = std::make_unique<tecs::test::TransformComponent::TransformConfig>();
    transform_config->x = kPosX;
    transform_config->y = kPosY;

    // Add transform component to entity using the entity handle
    bool add_transform_result = entity_handle.AddComponent<tecs::test::TransformComponent>(std::move(transform_config));
    EXPECT_TRUE(add_transform_result);

    // Check entity has the transform component using the entity handle
    bool has_transform = entity_handle.HasComponent<tecs::test::TransformComponent>();
    EXPECT_TRUE(has_transform);

    // Get the transform component from the world using the entity handle
    tecs::test::TransformComponent* transform_component = entity_handle.GetComponent<tecs::test::TransformComponent>();
    EXPECT_NE(transform_component, nullptr);

    // Verify the position values
    EXPECT_FLOAT_EQ(transform_component->GetX(), kPosX);
    EXPECT_FLOAT_EQ(transform_component->GetY(), kPosY);

    // Remove the transform component using the entity handle
    bool remove_transform_result = entity_handle.RemoveComponent<tecs::test::TransformComponent>();
    EXPECT_TRUE(remove_transform_result);

    // Check entity no longer has the transform component using the entity handle
    bool has_transform_after_removal = entity_handle.HasComponent<tecs::test::TransformComponent>();
    EXPECT_FALSE(has_transform_after_removal);

    // Destroy the entity using the entity handle
    bool destroy_result = entity_handle.Destroy();
    EXPECT_TRUE(destroy_result);

    // Verify the entity is no longer valid
    EXPECT_FALSE(world.CheckEntityValidity(entity));
}

TEST(tecs, use_system)
{
    // Get cpu core count
    uint32_t num_cores = std::thread::hardware_concurrency();
    EXPECT_NE(num_cores, 0);

    // Create job scheduler
    tecs::JobScheduler job_sched(num_cores);

    // Create sample system
    tecs::test::SampleSystem sample_system(job_sched);

    // Create singleton system proxy manager
    tecs::SystemProxyManager system_proxy_manager;

    {
        // Create system proxy for sample system
        tecs::SystemProxy sample_proxy(sample_system);

        // Register sample system proxy
        system_proxy_manager.RegisterSystemProxy<tecs::test::SampleSystem>(
            std::make_unique<tecs::SystemProxy>(sample_system));
    }

    {
        // Create task list
        tecs::System::TaskList task_list;

        // Modify the sample context data in the sample system using a task
        tecs::AddTask<tecs::test::SampleContext>(
            task_list, [this](tecs::test::SampleContext& ctx, tecs::JobScheduler& job_sched) -> bool
        {
            ctx.ModifyData(job_sched, tecs::test::kModifiedValue);
            return true; // Task executed successfully
        });

        // Get system proxy for sample system
        std::unique_ptr<tecs::SystemProxy> sample_proxy =
            system_proxy_manager.GetSystemProxy<tecs::test::SampleSystem>();

        // Submit task list to the system via proxy
        sample_proxy->SumbitTaskList(std::move(task_list));
    }

    // Run system update phases
    ASSERT_TRUE(sample_system.PreUpdate());
    ASSERT_TRUE(sample_system.Update());
    ASSERT_TRUE(sample_system.PostUpdate());

    {
        // Get system proxy for sample system
        std::unique_ptr<tecs::SystemProxy> sample_proxy =
            system_proxy_manager.GetSystemProxy<tecs::test::SampleSystem>();

        // Verify that the context data was modified by the task
        const tecs::test::SampleContext* context = sample_proxy->GetContext<tecs::test::SampleContext>();
        EXPECT_EQ(context->GetData(), tecs::test::kModifiedValue);
    }
}

TEST(tecs, use_ecs)
{
    // Get cpu core count
    uint32_t num_cores = std::thread::hardware_concurrency();
    EXPECT_NE(num_cores, 0);

    // Create job scheduler
    tecs::JobScheduler job_sched(num_cores);

    // Create sample system
    tecs::test::SampleSystem sample_system(job_sched);

    // Create singleton system proxy manager
    tecs::SystemProxyManager system_proxy_manager;

    {
        // Create system proxy for sample system
        tecs::SystemProxy sample_proxy(sample_system);

        // Register sample system proxy
        system_proxy_manager.RegisterSystemProxy<tecs::test::SampleSystem>(
            std::make_unique<tecs::SystemProxy>(sample_system));
    }

    // Create a world instance
    tecs::World world;

    // Create an entity handle
    std::unique_ptr<tecs::EntityHandle> entity_handle = nullptr;
    {
        // Create an entity
        tecs::Entity entity = world.CreateEntity();
        EXPECT_TRUE(world.CheckEntityValidity(entity));

        // Create an entity handle for the created entity
        entity_handle = std::make_unique<tecs::EntityHandle>(world, entity);
    }

    // Create behavior component config
    std::unique_ptr<tecs::test::BehaviorComponent::BehaviorConfig> behavior_config 
        = std::make_unique<tecs::test::BehaviorComponent::BehaviorConfig>();
    behavior_config->is_active = true;
    behavior_config->behavior_name = "TestBehavior";

    // Add behavior component to entity using the entity handle
    bool add_behavior_result = entity_handle->AddComponent<tecs::test::BehaviorComponent>(std::move(behavior_config));
    EXPECT_TRUE(add_behavior_result);

    // Commit the entity using entity handle
    bool commit_result = entity_handle->Commit();
    EXPECT_TRUE(commit_result);

    // Iterate through entities which have the BehaviorComponent
    for (const auto& entity : world.View<tecs::test::BehaviorComponent>())
    {
        // Create entity handle for the entity
        tecs::EntityHandle entity_handle_for(world, entity);

        // Get the behavior component from the world using the entity handle
        tecs::test::BehaviorComponent* behavior_component = entity_handle_for.GetComponent<tecs::test::BehaviorComponent>();
        EXPECT_NE(behavior_component, nullptr);

        // Update the behavior component
        behavior_component->Update(entity_handle_for, 0.016f); // Assuming 60
    }

    // Run system update phases
    ASSERT_TRUE(sample_system.PreUpdate());
    ASSERT_TRUE(sample_system.Update());
    ASSERT_TRUE(sample_system.PostUpdate());

    {
        // Get system proxy for sample system
        std::unique_ptr<tecs::SystemProxy> sample_proxy =
            system_proxy_manager.GetSystemProxy<tecs::test::SampleSystem>();

        // Verify that the context data was modified by the task
        const tecs::test::SampleContext* context = sample_proxy->GetContext<tecs::test::SampleContext>();
        EXPECT_EQ(context->GetData(), tecs::test::kModifiedValue);
    }
    
}