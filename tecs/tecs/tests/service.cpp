// Google Test
#include <gtest/gtest.h>

// TECS
#include "tecs/tecs.h"

namespace tecs::test
{

/**
 * @brief
 * Sample implementation of Service::Context for testing purposes
 */
class SampleContext : public Service::Context
{
public:
    SampleContext() = default;
    ~SampleContext() override = default;
};

} // namespace tecs::test

TEST(tecs, use_service)
{

}