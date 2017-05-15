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
#include "test_util/Producer_Consumer.cpp"

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

    std::vector<FieldVValues> downstream;
    std::vector<FieldVValues> upstream;

};

TEST_F(DriverFixture, TestSetUpAndTearDown) {}

//TEST_F(DriverFixture, TestSetToggleTraffic) {
//
//  for (unsigned int i = 0; i < bdpars::LastRegId; i++) {
//    driver->SetToggleTraffic(0, 
//}
