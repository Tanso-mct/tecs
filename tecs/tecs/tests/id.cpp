// Google Test
#include <gtest/gtest.h>

// TECS
#include "tecs/tecs.h"

TEST(tecs, id)
{
    // Create several ID instances for testing
    tecs::ID id1(1, 0);
    tecs::ID id2(1, 0);
    tecs::ID id3(2, 0);
    tecs::ID id4(1, 1);
    tecs::ID id5;

    // Test equality
    EXPECT_TRUE(id1 == id2);
    EXPECT_FALSE(id1 == id3);
    EXPECT_FALSE(id1 == id4);

    // Test inequality
    EXPECT_TRUE(id1 != id3);
    EXPECT_TRUE(id1 != id4);

    // Test less-than operator
    EXPECT_TRUE(id1 < id3);
    EXPECT_TRUE(id1 < id4);
    EXPECT_FALSE(id3 < id1);
    EXPECT_FALSE(id4 < id1);

    // Test validity
    EXPECT_TRUE(id1.IsValid());
    EXPECT_FALSE(id5.IsValid());
}