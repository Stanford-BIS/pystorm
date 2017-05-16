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

TEST_F(BDStateFixture, TestTrafficIsNotOffOnInit) 
{
  // we haven't set anything, traffic can't be off
  ASSERT_FALSE(state->IsTrafficOff());
}

TEST_F(BDStateFixture, TestTrafficTurnsOff) 
{
  // we haven't set anything, traffic can't be off
  ASSERT_FALSE(state->IsTrafficOff());

  const std::vector<bdpars::RegId> kTrafficRegs = {bdpars::NeuronDumpToggle, bdpars::TOGGLE_PRE_FIFO, bdpars::TOGGLE_POST_FIFO0, bdpars::TOGGLE_POST_FIFO1};
  for (auto& reg : kTrafficRegs) {
    state->SetToggle(reg, false, false);
  }

  // XXX there's a race here, we won't check this if the delay is set short enough
  if (driver_pars->Get(driverpars::bd_state_traffic_drain_us) > 100) {
    // an immediate query should return false still because of the delay
    ASSERT_FALSE(state->IsTrafficOff());
  }

  state->WaitForTrafficOff();
  ASSERT_TRUE(state->IsTrafficOff());
  
}
