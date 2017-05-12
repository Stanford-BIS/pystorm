#include "DriverPars.h"

#include <iostream>
using std::cout;
using std::endl;

#include "gtest/gtest.h"

using namespace pystorm;
using namespace bddriver;

class DriverParsFixture : public testing::Test
{
  public:
    void SetUp() {
      pars = new DriverPars();
    }
    void TearDown() {
      delete pars;
    }

    DriverPars * pars;
};

TEST_F(DriverParsFixture, TestSetUpTearDown) {}
