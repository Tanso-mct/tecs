// Google Test
#include <gtest/gtest.h>

// TECS
#include "tecs/tecs.h"

namespace tecs_registry_test
{

class TestComponentConfig : public tecs::ComponentConfig
{
};

class TestComponent : public tecs::ComponentBase<TestComponent>
{
public:
    TestComponent() : tecs::ComponentBase<TestComponent>("TestComponent", GUID{0, 0, 0, 0}) {}

    bool Import(std::unique_ptr<tecs::ComponentConfig> config) override
    {
        return true;
    }

    std::unique_ptr<tecs::ComponentConfig> Export() const override
    {
        return std::make_unique<TestComponentConfig>();
    }

    bool Update(tecs::Actor actor) override
    {
        return true;
    }

};

class AnotherTestComponentConfig : public tecs::ComponentConfig
{
};

class AnotherTestComponent : public tecs::ComponentBase<AnotherTestComponent>
{
public:
    AnotherTestComponent() : tecs::ComponentBase<AnotherTestComponent>("AnotherTestComponent", GUID{1, 1, 1, 1}) {}

    bool Import(std::unique_ptr<tecs::ComponentConfig> config) override
    {
        return true;
    }

    std::unique_ptr<tecs::ComponentConfig> Export() const override
    {
        return std::make_unique<AnotherTestComponentConfig>();
    }

    bool Update(tecs::Actor actor) override
    {
        return true;
    }

};

} // namespace tecs_registry_test

TEST(tecs, entity)
{
    tecs::Entity entity1(1, 0);
    tecs::Entity entity2(1, 0);
    tecs::Entity entity3(2, 0);
    tecs::Entity entity4(2, 1);

    EXPECT_TRUE(entity1 == entity2);
    EXPECT_FALSE(entity1 != entity2);
    EXPECT_TRUE(entity1 != entity3);
    EXPECT_FALSE(entity1 == entity3);
    EXPECT_FALSE(entity3 == entity4);

    EXPECT_TRUE(entity1 < entity3);
    EXPECT_FALSE(entity3 < entity1);
    EXPECT_TRUE(entity3 < entity4);
}

TEST(tecs, component)
{
    // Create an instance of the test component
    tecs_registry_test::TestComponent test_component;
    tecs_registry_test::AnotherTestComponent another_test_component;

    // Verify the component's name, GUID, and runtime ID
    EXPECT_EQ(test_component.GetName(), "TestComponent");
    EXPECT_EQ(test_component.GetGUID(), (GUID{0, 0, 0, 0}));
    EXPECT_EQ(tecs_registry_test::TestComponent::GetRuntimeID(), 0);

    EXPECT_EQ(another_test_component.GetName(), "AnotherTestComponent");
    EXPECT_EQ(another_test_component.GetGUID(), (GUID{1, 1, 1, 1}));
    EXPECT_EQ(tecs_registry_test::AnotherTestComponent::GetRuntimeID(), 1);

    // Import and export the component configuration
    std::unique_ptr<tecs::ComponentConfig> config = std::make_unique<tecs_registry_test::TestComponentConfig>();
    EXPECT_TRUE(test_component.Import(std::move(config)));

    std::unique_ptr<tecs::ComponentConfig> exported_config = test_component.Export();
    EXPECT_NE(exported_config, nullptr);

    // Update the component with a dummy actor
    tecs::Entity entity(1, 0);
    tecs::Registry registry;
    tecs::Actor actor(entity, registry);
    EXPECT_TRUE(test_component.Update(actor));
}

TEST(tecs, registry)
{
    tecs::Registry registry;

    // Create an entity
    tecs::Entity entity = registry.CreateEntity();
    EXPECT_TRUE(registry.CheckEntityValidity(entity));

    // Add a test component to the entity
    std::unique_ptr<tecs::Component> test_component = std::make_unique<tecs_registry_test::TestComponent>();
    EXPECT_TRUE(registry.AddComponent(entity, tecs_registry_test::TestComponent::GetRuntimeID(), std::move(test_component)));
    EXPECT_TRUE(registry.HasComponent(entity, tecs_registry_test::TestComponent::GetRuntimeID()));
    
    // Retrieve the component types for the entity
    std::vector<uint32_t> component_types = registry.GetComponentTypes(entity);
    EXPECT_EQ(component_types.size(), 1);
    EXPECT_EQ(component_types[0], tecs_registry_test::TestComponent::GetRuntimeID());

    // Retrieve the component and verify its type
    tecs::Component* retrieved_component 
        = registry.GetComponent(entity, tecs_registry_test::TestComponent::GetRuntimeID());
    EXPECT_NE(retrieved_component, nullptr);
    EXPECT_EQ(retrieved_component->GetName(), "TestComponent");

    // Retrieve the component using the const version of GetComponent
    const tecs::Component* const_retrieved_component 
        = registry.GetComponent(entity, tecs_registry_test::TestComponent::GetRuntimeID());
    EXPECT_NE(const_retrieved_component, nullptr);
    EXPECT_EQ(const_retrieved_component->GetName(), "TestComponent");

    // View the entities with the test component but without committing the entity
    const std::set<tecs::Entity>& entities_with_test_component 
        = registry.View(tecs_registry_test::TestComponent::GetRuntimeID());
    EXPECT_TRUE(entities_with_test_component.empty());

    // Commit the entity
    EXPECT_TRUE(registry.CommitEntity(entity));

    // View the entities with the test component
    for (const tecs::Entity& e : registry.View<tecs_registry_test::TestComponent>())
        EXPECT_EQ(e, entity);

    // Destroy the entity
    EXPECT_TRUE(registry.DestroyEntity(entity));
    EXPECT_FALSE(registry.CheckEntityValidity(entity));
}

TEST(tecs, actor)
{
    tecs::Registry registry;

    // Create an entity
    tecs::Entity entity = registry.CreateEntity();
    EXPECT_TRUE(registry.CheckEntityValidity(entity));

    // Create an actor for the entity
    tecs::Actor actor(entity, registry);
    EXPECT_TRUE(actor.IsValid());

    // Add a test component to the actor
    std::unique_ptr<tecs::Component> test_component = std::make_unique<tecs_registry_test::TestComponent>();
    EXPECT_TRUE(actor.AddComponent<tecs_registry_test::TestComponent>(std::move(test_component)));
    EXPECT_TRUE(actor.HasComponent<tecs_registry_test::TestComponent>());

    // Retrieve the component from the actor and verify its type
    tecs::Component* retrieved_component = actor.GetComponent<tecs_registry_test::TestComponent>();
    EXPECT_NE(retrieved_component, nullptr);
    EXPECT_EQ(retrieved_component->GetName(), "TestComponent");

    // Retrieve the component using the const version of GetComponent
    const tecs::Component* const_retrieved_component = actor.GetComponent<tecs_registry_test::TestComponent>();
    EXPECT_NE(const_retrieved_component, nullptr);
    EXPECT_EQ(const_retrieved_component->GetName(), "TestComponent");

    // Remove the component from the actor
    EXPECT_TRUE(actor.RemoveComponent<tecs_registry_test::TestComponent>());
    EXPECT_FALSE(actor.HasComponent<tecs_registry_test::TestComponent>());
}