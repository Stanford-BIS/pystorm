#include "model/BDModelDriver.h"
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


class DriverFixture : public testing::Test {
  public:
    void SetUp() {

      //driver = Driver::GetInstance();
      driver = new BDModelDriver();
      model = driver->GetBDModel();

      driver->Start();
      driver->InitBD();
    }

    void TearDown() {
      // have to wait for everything to resolve!
      std::this_thread::sleep_for(std::chrono::seconds(kSleepS));
      
      // compare states for every test!
      driver->Stop();

      // compare the state
      // lock/unlock should be unecessary: we stopped the driver
      const BDState * model_state = model->LockState();
      ASSERT_EQ(*model_state, *driver->GetState(kCoreId));
      model->UnlockState();

      // compare the stuff that isn't captured by state
      ASSERT_EQ(model->PopSpikes(), sent_spikes); // make sure the spikes came through OK
      ASSERT_EQ(model->PopTags(), sent_tags); // make sure the spikes came through OK

      delete driver;
    }

    const unsigned int kSleepS = 1;
    const unsigned int M = 64; // be careful, keep it smaller than the memories, the PAT has 64 entries
    const unsigned kCoreId = 0;
    BDModelDriver * driver;
    bdmodel::BDModel * model;

    std::vector<SynSpike> sent_spikes;
    std::vector<Tag> sent_tags;

    void SendSpikes() {
      std::vector<SynSpike> spikes = MakeRandomSynSpikes(M, std::vector<unsigned int>(M, 0), std::vector<unsigned int>(M, kCoreId));
      driver->SendSpikes(spikes);
      sent_spikes.insert(sent_spikes.end(), spikes.begin(), spikes.end());
    }

    void SendTags() {
      std::vector<Tag> tags = MakeRandomTags(M, std::vector<unsigned int>(M, 0), std::vector<unsigned int>(M, kCoreId));
      driver->SendTags(tags);
      sent_tags.insert(sent_tags.end(), tags.begin(), tags.end());
    }

};

//TEST_F(DriverFixture, TestSetUpAndTearDown) {}

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

TEST_F(DriverFixture, TestSendTags) {
  SendTags();
}

TEST_F(DriverFixture, TestDownStreamCalls) {
  driver->SetPAT(kCoreId, MakeRandomPATData(M), 0);
  driver->SetTAT(kCoreId, 0, MakeRandomTATData(M), 0);
  driver->SetTAT(kCoreId, 1, MakeRandomTATData(M), 0);
  driver->SetAM(kCoreId, MakeRandomAMData(M), 0);
  driver->SetMM(kCoreId, MakeRandomMMData(M), 0);
  SendSpikes();
}

