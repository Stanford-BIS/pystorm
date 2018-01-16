#include "BDState.h"
#include "common/DriverPars.h"

#include <iostream>
using std::cout;
using std::endl;

#include "gtest/gtest.h"

using namespace pystorm;
using namespace bddriver;

class BDStateFixture : public testing::Test {
 public:
  void SetUp() {
    bd_pars     = new bdpars::BDPars();
    state       = new BDState(bd_pars);
  }
  void TearDown() {
    delete bd_pars;
    delete state;
  }

  BDState* state;
  bdpars::BDPars* bd_pars;
};

TEST_F(BDStateFixture, TestSetUpTearDown) {}

TEST_F(BDStateFixture, TestTrafficIsNotOffOnInit) {
  // we haven't set anything, traffic can't be off
  ASSERT_FALSE(state->IsTrafficOff());
}

TEST_F(BDStateFixture, TestTrafficTurnsOff) {
  // we haven't set anything, traffic can't be off
  ASSERT_FALSE(state->IsTrafficOff());

  const std::vector<bdpars::BDHornEP> kTrafficRegs = {
      bdpars::BDHornEP::NEURON_DUMP_TOGGLE, bdpars::BDHornEP::TOGGLE_PRE_FIFO, bdpars::BDHornEP::TOGGLE_POST_FIFO0, bdpars::BDHornEP::TOGGLE_POST_FIFO1};
  for (auto& reg : kTrafficRegs) {
    state->SetToggle(reg, false, false);
  }

  // XXX there's a race here, we won't check this if the delay is set short enough
  if (driverpars::BD_STATE_TRAFFIC_DRAIN_US > 100) {
    // an immediate query should return false still because of the delay
    ASSERT_FALSE(state->IsTrafficOff());
  }

  state->WaitForTrafficOff();
  ASSERT_TRUE(state->IsTrafficOff());
}
