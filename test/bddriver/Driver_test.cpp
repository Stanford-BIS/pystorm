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

constexpr uint64_t     TATAccWord::field_hard_values[];
constexpr unsigned int TATAccWord::field_widths[];

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

    std::vector<BDWord> sent_spikes;
    std::vector<BDWord> sent_tags;

    void SendSpikes() {
      std::vector<BDWord> spikes = MakeRandomSynSpikes(M);
      std::vector<unsigned int> core_ids(spikes.size(), 0);
      std::vector<BDTime> times(spikes.size(), 0);
      driver->SendSpikes(core_ids, spikes, times);
      sent_spikes.insert(sent_spikes.end(), spikes.begin(), spikes.end());
    }

    void SendTags() {
      std::vector<BDWord> tags = MakeRandomInputTags(M);
      std::vector<unsigned int> core_ids(tags.size(), 0);
      std::vector<BDTime> times(tags.size(), 0);
      driver->SendTags(core_ids, tags, times);
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
  driver->SetMem(kCoreId, bdpars::PAT, MakeRandomPATData(M), 0);
}

TEST_F(DriverFixture, TestSetTAT0) {
  driver->SetMem(kCoreId, bdpars::TAT0, MakeRandomTATData(M), 0);
}

TEST_F(DriverFixture, TestSetTAT1) {
  driver->SetMem(kCoreId, bdpars::TAT1, MakeRandomTATData(M), 0);
}

TEST_F(DriverFixture, TestSetAM) {
  driver->SetMem(kCoreId, bdpars::AM, MakeRandomAMData(M), 0);
}

TEST_F(DriverFixture, TestSetMM) {
  driver->SetMem(kCoreId, bdpars::MM, MakeRandomMMData(M), 0);
}

TEST_F(DriverFixture, TestSendSpikes) {
  SendSpikes();
}

TEST_F(DriverFixture, TestSendTags) {
  SendTags();
}

TEST_F(DriverFixture, TestDownStreamCalls) {
  driver->SetMem(kCoreId, bdpars::PAT, MakeRandomPATData(M), 0);
  driver->SetMem(kCoreId, bdpars::TAT0, MakeRandomTATData(M), 0);
  driver->SetMem(kCoreId, bdpars::TAT1, MakeRandomTATData(M), 0);
  driver->SetMem(kCoreId, bdpars::AM, MakeRandomAMData(M), 0);
  driver->SetMem(kCoreId, bdpars::MM, MakeRandomMMData(M), 0);
  SendSpikes();
}

// upstream-downstream tests

// for dump tests, need to program before dumping

TEST_F(DriverFixture, TestDumpPAT) {
  unsigned int size = driver->GetBDPars()->Size(bdpars::PAT);
  auto data = MakeRandomPATData(size);
  driver->SetMem(kCoreId, bdpars::PAT, data, 0);
  auto dumped = driver->DumpMem(kCoreId, bdpars::PAT);
  ASSERT_EQ(data, dumped);
}

TEST_F(DriverFixture, TestDumpAM) {
  unsigned int size = driver->GetBDPars()->Size(bdpars::AM);
  auto data = MakeRandomAMData(size);
  driver->SetMem(kCoreId, bdpars::AM, data, 0);
  auto dumped = driver->DumpMem(kCoreId, bdpars::AM);
  ASSERT_EQ(data, dumped);
}

TEST_F(DriverFixture, TestDumpMM) {
  unsigned int size = driver->GetBDPars()->Size(bdpars::MM);
  auto data = MakeRandomMMData(size);
  driver->SetMem(kCoreId, bdpars::MM, data, 0);
  auto dumped = driver->DumpMem(kCoreId, bdpars::MM);
  ASSERT_EQ(data, dumped);
}

TEST_F(DriverFixture, TestDumpTAT0) {
  unsigned int size = driver->GetBDPars()->Size(bdpars::TAT0);
  auto data = MakeRandomTATData(size);
  driver->SetMem(kCoreId, bdpars::TAT0, data, 0);
  auto dumped = driver->DumpMem(kCoreId, bdpars::TAT0);
  ASSERT_EQ(data, dumped);
}

TEST_F(DriverFixture, TestDumpTAT1) {
  unsigned int size = driver->GetBDPars()->Size(bdpars::TAT1);
  auto data = MakeRandomTATData(size);
  driver->SetMem(kCoreId, bdpars::TAT1, data, 0);
  auto dumped = driver->DumpMem(kCoreId, bdpars::TAT1);
  ASSERT_EQ(data, dumped);
}

// upstream-only tests

TEST_F(DriverFixture, TestRecvSpikes) {
  auto to_send = MakeRandomNrnSpikes(M);
  model->PushOutput(bdpars::OUTPUT_SPIKES, to_send);
  std::this_thread::sleep_for(std::chrono::seconds(1));

  ASSERT_EQ(std::get<1>(driver->RecvSpikes(to_send.size())), to_send);
}

TEST_F(DriverFixture, TestRecvTags) {
  auto to_send = MakeRandomInputTags(M);
  model->PushOutput(bdpars::TAT_OUTPUT_TAGS, to_send); // XXX not testing acc, has a smaller gtag width, would have to limit gtag size
  std::this_thread::sleep_for(std::chrono::seconds(1));

  ASSERT_EQ(std::get<1>(driver->RecvTags(to_send.size())), to_send);
}

TEST_F(DriverFixture, TestGetPreFIFOTags) {
  auto to_send = MakeRandomPreFIFOTags(M);
  model->PushOutput(bdpars::PRE_FIFO_TAGS, to_send); 
  std::this_thread::sleep_for(std::chrono::seconds(1));

  ASSERT_EQ(driver->GetPreFIFODump(kCoreId, to_send.size()), to_send);
}

TEST_F(DriverFixture, TestGetPostFIFOTags) {
  std::vector<BDWord> to_send0, to_send1;
  to_send0 = MakeRandomPostFIFOTags(M);
  to_send1 = MakeRandomPostFIFOTags(M);

  // try PostFIFO0
  model->PushOutput(bdpars::POST_FIFO_TAGS0, to_send0); 

  std::this_thread::sleep_for(std::chrono::seconds(1));

  ASSERT_EQ(driver->GetPostFIFODump(kCoreId, to_send0.size(), 0).first, to_send0);

  // try PostFIFO
  model->PushOutput(bdpars::POST_FIFO_TAGS1, to_send1); 
  std::this_thread::sleep_for(std::chrono::seconds(1));

  ASSERT_EQ(driver->GetPostFIFODump(kCoreId, 0, to_send1.size()).second, to_send1);
}

TEST_F(DriverFixture, TestFIFOOverflow) {
  const std::pair<unsigned int, unsigned int> to_push = {10, 20};
  model->PushOutput(bdpars::OVERFLOW_TAGS0, std::vector<BDWord>(to_push.first, BDWord(1)));
  model->PushOutput(bdpars::OVERFLOW_TAGS1, std::vector<BDWord>(to_push.second, BDWord(1)));

  std::this_thread::sleep_for(std::chrono::seconds(1));

  ASSERT_EQ(driver->GetFIFOOverflowCounts(kCoreId), to_push);
}


