#include <gtest/gtest.h>
#include "tecs/tecs.h"

TEST(tecs, smoke)
{
    tecs::Sample sample;
    sample.DisplayMessage();
    SUCCEED();
}