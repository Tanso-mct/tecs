// Google test
#include <gtest/gtest.h>

// TECS
#include <tecs/tecs.h>

// Platform Window
#include "platform_window/platform_window.h"

TEST(platform_window, sample_hello)
{
    platform::window::Sample sample;
    sample.Hello();
}