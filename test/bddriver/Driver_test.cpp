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
    DriverFixture() {
      //driver = Driver::GetInstance();
      driver = new BDModelDriver();
      model = driver->GetBDModel();
    }

    void SetUp() {
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
    }

    ~DriverFixture() {
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

// Downstream-only tests (almost, programing AM has an upstream component)

TEST_F(DriverFixture, TestInitBD) {}

TEST_F(DriverFixture, TestSetPreFIFODumpStateOn) {
  driver->SetPreFIFODumpState(kCoreId, true);
}

TEST_F(DriverFixture, TestSetPostFIFODumpStateOn) {
  driver->SetPostFIFODumpState(kCoreId, true);
}

TEST_F(DriverFixture, TestSetPreFIFODumpStateOnOff) {
  driver->SetPreFIFODumpState(kCoreId, true);
  driver->SetPreFIFODumpState(kCoreId, false);
}

TEST_F(DriverFixture, TestSetPostFIFODumpStateOnOff) {
  driver->SetPostFIFODumpState(kCoreId, true);
  driver->SetPostFIFODumpState(kCoreId, false);
}

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

// upstream-downstream tests

// for dump tests, need to program before dumping

TEST_F(DriverFixture, TestDumpPAT) {
  unsigned int size = driver->GetBDPars()->Size(bdpars::PAT);
  auto data = MakeRandomPATData(size);
  driver->SetPAT(kCoreId, data, 0);
  auto dumped = driver->DumpPAT(kCoreId);
  ASSERT_EQ(data, dumped);
}

TEST_F(DriverFixture, TestDumpAM) {
  unsigned int size = driver->GetBDPars()->Size(bdpars::AM);
  auto data = MakeRandomAMData(size);
  driver->SetAM(kCoreId, data, 0);
  auto dumped = driver->DumpAM(kCoreId);
  ASSERT_EQ(data, dumped);
}

TEST_F(DriverFixture, TestDumpMM) {
  unsigned int size = driver->GetBDPars()->Size(bdpars::MM);
  auto data = MakeRandomMMData(size);
  driver->SetMM(kCoreId, data, 0);
  auto dumped = driver->DumpMM(kCoreId);
  ASSERT_EQ(data, dumped);
}

TEST_F(DriverFixture, TestDumpTAT0) {
  unsigned int size = driver->GetBDPars()->Size(bdpars::TAT0);
  auto data = MakeRandomTATData(size);
  driver->SetTAT(kCoreId, 0, data, 0);
  auto dumped = driver->DumpTAT(kCoreId, 0);
  ASSERT_EQ(data, dumped);
}

TEST_F(DriverFixture, TestDumpTAT1) {
  unsigned int size = driver->GetBDPars()->Size(bdpars::TAT1);
  auto data = MakeRandomTATData(size);
  driver->SetTAT(kCoreId, 1, data, 0);
  auto dumped = driver->DumpTAT(kCoreId, 1);
  ASSERT_EQ(data, dumped);
}

// upstream-only tests

TEST_F(DriverFixture, TestRecvSpikes) {
  auto to_send = MakeRandomNrnSpikes(M, std::vector<unsigned int>(M, 0), std::vector<unsigned int>(M, kCoreId));
  model->PushUpstreamSpikes(to_send);
  std::this_thread::sleep_for(std::chrono::seconds(1));

  ASSERT_EQ(driver->RecvSpikes(to_send.size()), to_send);
}

TEST_F(DriverFixture, TestRecvTags) {
  auto to_send = MakeRandomTags(M, std::vector<unsigned int>(M, 0), std::vector<unsigned int>(M, kCoreId));
  model->PushUpstreamTATTags(to_send); // XXX not testing acc, has a smaller gtag width, would have to limit gtag size
  std::this_thread::sleep_for(std::chrono::seconds(1));

  ASSERT_EQ(driver->RecvTags(to_send.size()), to_send);
}

TEST_F(DriverFixture, TestGetPreFIFOTags) {
  auto to_send = MakeRandomTags(M, std::vector<unsigned int>(M, 0), std::vector<unsigned int>(M, kCoreId));
  model->PushPreFIFOTags(to_send); 
  std::this_thread::sleep_for(std::chrono::seconds(1));

  ASSERT_EQ(driver->GetPreFIFODump(kCoreId, to_send.size()), to_send);
}

TEST_F(DriverFixture, TestGetPostFIFOTags) {
  auto to_send_orig = MakeRandomTags(M, std::vector<unsigned int>(M, 0), std::vector<unsigned int>(M, kCoreId));
  std::vector<Tag> to_send0, to_send1;
  // sort into class 0 and class 1 tags:
  for (auto& tag : to_send_orig) {
    Tag mod_tag = tag;
    mod_tag.tag = mod_tag.tag % driver->GetBDPars()->Size(bdpars::FIFO_DCT) / 2;
    to_send0.push_back(mod_tag);

    mod_tag.tag = mod_tag.tag + driver->GetBDPars()->Size(bdpars::FIFO_DCT) / 2;
    to_send1.push_back(mod_tag);
  }

  model->PushPostFIFOTags(to_send0, 0); // try PostFIFO0 first
  std::this_thread::sleep_for(std::chrono::seconds(1));

  ASSERT_EQ(driver->GetPostFIFODump(kCoreId, to_send0.size(), 0), to_send0);

  // still send to_send0, use to_send1 to compare
  model->PushPostFIFOTags(to_send0, 1); // try PostFIFO1 now
  std::this_thread::sleep_for(std::chrono::seconds(1));

  ASSERT_EQ(driver->GetPostFIFODump(kCoreId, 0, to_send0.size()), to_send1);
}

TEST_F(DriverFixture, TestFIFOOverflow) {
  const std::pair<unsigned int, unsigned int> to_push = {10, 20};
  model->PushFIFOOverflow(to_push.first, 0);
  model->PushFIFOOverflow(to_push.second, 1);

  std::this_thread::sleep_for(std::chrono::seconds(1));

  ASSERT_EQ(driver->GetFIFOOverflowCounts(kCoreId), to_push);
}


