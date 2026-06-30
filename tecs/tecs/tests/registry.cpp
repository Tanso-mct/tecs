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

} // namespace tecs_registry_test

TEST(tesc, entity)
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