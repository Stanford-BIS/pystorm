#include "BDState.h"

#include <iostream>
using std::cout;
using std::endl;

#include "gtest/gtest.h"

using namespace pystorm;
using namespace bddriver;

class BDStateFixture : public testing::Test
{
  public:
    void SetUp() {
      bd_pars = new bdpars::BDPars();
      driver_pars = new driverpars::DriverPars();
      state = new BDState(bd_pars, driver_pars);
    }
    void TearDown() {
      delete bd_pars;
      delete driver_pars;
      delete state;
    }

    BDState * state;
    bdpars::BDPars * bd_pars;
    driverpars::DriverPars * driver_pars;
};

TEST_F(BDStateFixture, TestSetUpTearDown) {}
