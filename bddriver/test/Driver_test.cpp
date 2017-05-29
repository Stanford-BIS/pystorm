#include "Driver.h"
#include "BDModel.h"

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

#include <chrono>
#include <thread>

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

      //driver = Driver::GetInstance();
      driver = new Driver();

      model = new bdmodel::BDModel(driver->GetBDPars(), driver->GetDriverPars());

      driver->Start();
    }

    void TearDown() {
      // have to wait for everything to resolve!
      std::this_thread::sleep_for(std::chrono::seconds(kSleepS));
      
      // compare states for every test!

      // read soft comm's output file (the raw data that would get sent over USB)
      const std::string * fname = driver->GetDriverPars()->Get(driverpars::soft_comm_out_fname);
      std::ifstream file(*fname, ios::in|ios::binary|ios::ate);
      std::vector<char> usb_data;
      if (file.is_open()) {
        unsigned int size = file.tellg();
        usb_data.resize(size);
        file.seekg(0, ios::beg);
        file.read(&usb_data[0], size);
        file.close();
      } else {
        cout << "couldn't open the soft comm out file (" << fname << ") for some reason" << endl;
        ASSERT_TRUE(false);
      }

      //cout << "STREAM SIZE" << endl;
      //cout << usb_data.size() << endl;
      
      std::vector<uint8_t> usb_data_uint;
      for (char& char_el : usb_data) {
        usb_data_uint.push_back(static_cast<uint8_t>(char_el));
      }

      // parse the file with BDModel
      model->ParseInput(usb_data_uint);

      // compare the state
      ASSERT_EQ(*model->GetState(), *driver->GetState(kCoreId));

      // compare the stuff that isn't captured by state
      ASSERT_EQ(*model->GetSpikes(), sent_spikes); // make sure the spikes came through OK
      ASSERT_EQ(*model->GetTags(), sent_tags); // make sure the spikes came through OK
      
      driver->Stop();

      delete driver;
      delete model;
    }

    const unsigned int kSleepS = 1;
    const unsigned int M = 64; // be careful, keep it smaller than the memories, the PAT has 64 entries
    const unsigned kCoreId = 0;
    Driver * driver;
    bdmodel::BDModel * model;

    std::vector<SynSpike> sent_spikes;
    std::vector<Tag> sent_tags;

    void SendSpikes() {
      std::vector<SynSpike> spikes = MakeRandomSynSpikes(M, std::vector<unsigned int>(M, 0), std::vector<unsigned int>(M, kCoreId));
      driver->SendSpikes(spikes);
      sent_spikes.insert(sent_spikes.end(), spikes.begin(), spikes.end());
    }

};

//TEST_F(DriverFixture, TestSetUpAndTearDown) {}
//
//TEST_F(DriverFixture, TestDownStreamCalls) {
//  // just make a bunch of downstream calls. Make sure we don't die.
//  // XXX no correctness testing yet
//  driver->SetPAT(kCoreId, MakeRandomPATData(M), 0);
//  //cout << "set PAT" << endl;
//  driver->SetTAT(kCoreId, 0, MakeRandomTATData(M), 0);
//  //cout << "set TAT0" << endl;
//  driver->SetTAT(kCoreId, 1, MakeRandomTATData(M), 0);
//  //cout << "set TAT1" << endl;
//  driver->SetAM(kCoreId, MakeRandomAMData(M), 0);
//  //cout << "set AM" << endl;
//  driver->SetMM(kCoreId, MakeRandomMMData(M), 0);
//  //cout << "set MM" << endl;
//  driver->SendSpikes(MakeRandomSynSpikesSameCoreId(M, kCoreId));
//  //cout << "sent some spikes" << endl;
//}

TEST_F(DriverFixture, TestSetPAT) {
  driver->SetPAT(kCoreId, MakeRandomPATData(M), 0);
}

TEST_F(DriverFixture, TestSetTAT0) {
  driver->SetTAT(kCoreId, 0, MakeRandomTATData(M), 0);
}

TEST_F(DriverFixture, TestSetTAT1) {
  driver->SetTAT(kCoreId, 1, MakeRandomTATData(M), 0);
}

TEST_F(DriverFixture, TestSetAM) {
  driver->SetAM(kCoreId, MakeRandomAMData(M), 0);
}

TEST_F(DriverFixture, TestSetMM) {
  driver->SetMM(kCoreId, MakeRandomMMData(M), 0);
}

TEST_F(DriverFixture, TestSendSpikes) {
  SendSpikes();
}
