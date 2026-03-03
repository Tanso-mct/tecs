// Google test
#include <gtest/gtest.h>

// TECS
#include <tecs/tecs.h>

// Platform
#include "platform/window_container.h"
using namespace tecs::game;

TEST(tecs_game_platform, window_container)
{
    platform::WindowContainer container;

    // Create a dummy window
    class DummyWindow : public platform::Window {};
    auto window1 = std::make_unique<DummyWindow>();
    auto window2 = std::make_unique<DummyWindow>();

    // Create IDs for the windows
    tecs::ID id1(1, 0);
    tecs::ID id2(2, 0);

    // Add windows to the container
    container.AddWindow(id1, std::move(window1));
    container.AddWindow(id2, std::move(window2));

    // Get windows from the container
    EXPECT_NE(container.GetWindow(id1), nullptr);
    EXPECT_NE(container.GetWindow(id2), nullptr);

    // Get window count
    EXPECT_EQ(container.GetWindowCount(), 2);
}
