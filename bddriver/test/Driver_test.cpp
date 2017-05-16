#include "Driver.h"

#include <chrono>
#include <cstdint>
#include <random>
#include <vector>
#include <iostream>
#include <cstdio>
#include <fstream>

#include "binary_util.h"
#include "gtest/gtest.h"
#include "test_util/DriverTypes_util.h"
#include "test_util/Producer_Consumer.h"

using std::cout;
using std::endl;
using namespace pystorm;
using namespace bddriver;


class DriverFixture : public testing::Test
{
  public:
    void SetUp() {

      // we need to make sure that the comm input file exists
      driverpars::DriverPars tmp_pars; // just want the SoftComm input file name
      const std::string * comm_in_fname = tmp_pars.Get(driverpars::soft_comm_in_fname);
      std::ofstream comm_input_file(*comm_in_fname);
      comm_input_file.close();

      driver = Driver::GetInstance();

      driver->Start();
    }

    void TearDown() {
      driver->Stop();
    }

    const unsigned kCoreId = 0;
    Driver * driver;


};

TEST_F(DriverFixture, TestSetUpAndTearDown) {}

TEST_F(DriverFixture, TestDownStreamCalls) {
  // just make a bunch of downstream calls. Make sure we don't die.
  // XXX no correctness testing yet
  unsigned int M = 10;

  driver->SetPAT(kCoreId, MakeRandomPATData(M), 0);
  //cout << "set PAT" << endl;
  driver->SetTAT(kCoreId, 0, MakeRandomTATData(M), 0);
  //cout << "set TAT0" << endl;
  driver->SetTAT(kCoreId, 1, MakeRandomTATData(M), 0);
  //cout << "set TAT1" << endl;
  driver->SetAM(kCoreId, MakeRandomAMData(M), 0);
  //cout << "set AM" << endl;
  driver->SetMM(kCoreId, MakeRandomMMData(M), 0);
  //cout << "set MM" << endl;
  driver->SendSpikes(MakeRandomSpikesSameCoreId(M, kCoreId));
  //cout << "sent some spikes" << endl;
}
