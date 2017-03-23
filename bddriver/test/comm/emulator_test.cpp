#include "Emulator.h"
#include "gtest/gtest.h"

namespace pystorm
{
namespace bddriver
{
namespace bdcomm
{

class EmulatorFixture : protected Emulator, public testing::Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

// WIP Add tests
TEST_F(EmulatorFixture, testBuildInputStream)
{
    Start();
}

}
}
}
