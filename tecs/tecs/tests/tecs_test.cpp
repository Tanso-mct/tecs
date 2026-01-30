#include <gtest/gtest.h>
#include "tecs.h"

TEST(tecs, smoke)
{
    tecs::Sample sample;
    sample.DisplayMessage();
    SUCCEED();
}