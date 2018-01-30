#include "Driver.h"

#include <cassert>
#include <iostream>
#include <unordered_map>
#include <array>
#include <vector>
#include <queue>
#include <algorithm>
#include <memory>
#include <utility>
#include <chrono>
#include <thread>

#include "comm/Comm.h"
#include "comm/CommSoft.h"
#include "comm/Emulator.h"
#ifdef BD_COMM_TYPE_OPALKELLY
#include "comm/CommOK.h"
#elif BD_COMM_TYPE_USB
#include "comm/CommUSB.h"
#endif

#include "common/BDPars.h"
#include "common/BDWord.h"
#include "common/BDState.h"
#include "common/DriverPars.h"
#include "common/DriverTypes.h"
#include "common/MutexBuffer.h"
#include "common/vector_util.h"
#include "decoder/Decoder.h"
#include "encoder/Encoder.h"

using std::cout;
using std::endl;

namespace pystorm {
namespace bddriver {

// Driver * Driver::GetInstance()
//{
//    // In C++11, if control from two threads occurs concurrently, execution
//    // shall wait during static variable initialization, therefore, this is
//    // thread safe
//    static Driver m_instance;
//    return &m_instance;
//}

Driver::Driver() {
  // load parameters
  bd_pars_     = new bdpars::BDPars();
  ok_pars_     = OKPars();

  // one BDState object per core
  // bd_state_ = std::vector<BDState>(bd_pars_->NumCores, BDState(bd_pars_));
  for (unsigned int i = 0; i < bd_pars_->NumCores; i++) {
    bd_state_.push_back(BDState(bd_pars_));
  }

  // initialize buffers
  enc_buf_in_  = new MutexBuffer<EncInput>();
  enc_buf_out_ = new MutexBuffer<EncOutput>();
  dec_buf_in_  = new MutexBuffer<DecInput>();

  // there is one dec_buf_out per upstream EP
  std::vector<uint8_t> up_eps = bd_pars_->GetUpEPs();

  for (auto& it : up_eps) {
    dec_bufs_out_.insert({it, new MutexBuffer<DecOutput>()});
  }

  // initialize deserializers for upstream traffic
  // deserialization is needed when ep size exceeds FPGA word payload size
  // deserializers make it possible to track "extra" remainder words
  // we might get out of the decoder
  for (auto& ep : up_eps) {
    const unsigned int FPGA_payload_width = FieldWidth(FPGAIO::PAYLOAD);
    const unsigned int ep_data_size = bd_pars_->Up_EP_size_.at(ep);
    const unsigned int D = ep_data_size % FPGA_payload_width == 0 ?
        ep_data_size / FPGA_payload_width
      : ep_data_size / FPGA_payload_width + 1;

    // only create a deserializer when needed
    if (D > 1) {
      up_ep_deserializers_.insert({ep, new VectorDeserializer<DecOutput>(D)});
    }
  }

  // initialize Encoder and Decoder
  enc_ = new Encoder(
      enc_buf_in_,
      enc_buf_out_,
      bd_pars_,
      driverpars::ENC_TIMEOUT_US);

  dec_ = new Decoder(
      dec_buf_in_,
      dec_bufs_out_,
      bd_pars_,
      driverpars::DEC_TIMEOUT_US);

  // initialize Comm
#ifdef BD_COMM_TYPE_SOFT
  cout << "initializing CommSoft" << endl;
    comm_ = new comm::CommSoft(
        "soft_comm_in.dat",
        "soft_comm_out.dat",
        dec_buf_in_,
        enc_buf_out_);
#elif BD_COMM_TYPE_USB
  cout << "NOT initializing USB comm" << endl;
    assert(false && "libUSB Comm is not implemented");
#elif BD_COMM_TYPE_MODEL
  cout << "NOT initializing BDModelComm (yet)" << endl;
    comm_ = nullptr;
#elif BD_COMM_TYPE_OPALKELLY
  cout << "initializing OKComm" << endl;
    comm_ = new comm::CommOK(dec_buf_in_, enc_buf_out_);
#else
  cout << "NOT initializing UNHANDLED Comm type comm" << endl;
    assert(false && "unhandled comm_type");
#endif

  // OK appreciates a little sleep time?
  std::this_thread::sleep_for(std::chrono::microseconds(100));
  cout << "Driver constructor done" << endl;

  // set up FPGA data structures
  SG_en_.resize(bd_pars_->NumCores);

}

Driver::~Driver() {
  delete bd_pars_;
  delete enc_buf_in_;
  delete enc_buf_out_;
  delete dec_buf_in_;
  for (auto& it : dec_bufs_out_) {
    delete it.second;
  }
  for (auto& it : up_ep_deserializers_) {
    delete it.second;
  }
  delete enc_;
  delete dec_;
  delete comm_;
}

// XXX get rid of this, it's used in some test
void Driver::testcall(const std::string& msg) { std::cout << msg << std::endl; }

void Driver::SetTimeUnitLen(BDTime ns_per_unit) {

  // update FPGA state
  ns_per_unit_ = ns_per_unit;
  clks_per_unit_ = ns_per_unit / ns_per_clk_;
  //cout << "setting FPGA time unit to " << ns_per_unit << " ns = " << clks_per_unit_ << " clocks per unit" << endl;

  // make sure that we aren't going to break the SG or SF
  // XXX can check highest_SF/SG_used instead, emit harder error

  const unsigned int fudge = 200; // extra cycles to receive rate updates, warm up/cool down pipeline, etc.
  if (max_num_SG_ * clks_per_SG_ + fudge >= clks_per_unit_) {
    cout << "WARNING: ns_per_unit is very small: FPGA Spike Generator updates might not complete" << endl;
    cout << "  clks_per_unit_ was " << clks_per_unit_ << endl;
    cout << "  Spike Generator requires " << clks_per_SG_ << " cycles per operation" << endl;
    cout << "  Max Spike Generators: " << max_num_SG_ << endl;
  }

  if (max_num_SF_ * clks_per_SF_ + fudge >= clks_per_unit_) {
    cout << "WARNING: ns_per_unit is very small: FPGA Spike Filter updates might not complete" << endl;
    cout << "  clks_per_unit_ was " << clks_per_unit_ << endl;
    cout << "  Spike Filter requires " << clks_per_SF_ << " cycles per operation" << endl;
    cout << "  Max Spike Filters: " << max_num_SG_ << endl;
  }

  BDWord unit_len_word = PackWord<FPGATMUnitLen>({{FPGATMUnitLen::UNIT_LEN, clks_per_unit_}});
  SendToEP(0, bdpars::FPGARegEP::TM_UNIT_LEN, {unit_len_word}); // XXX core id?
  Flush();
}

void Driver::SetTimePerUpHB(BDTime ns_per_hb) {
  units_per_HB_ = NsToUnits(ns_per_hb);
  cout << "setting HB reporting period to " << ns_per_hb << " ns = " << units_per_HB_ << " FPGA time units" << endl;

  //if (ns_per_hb <= 100000) cout << "****************WARNING: <100 US PER HB SEEMS TO CAUSE PROBLEMS****************" << endl;

  BDWord units_per_HB_word = static_cast<uint64_t>(units_per_HB_);
  uint64_t w0 = GetField(units_per_HB_word, THREEFPGAREGS::W0);
  uint64_t w1 = GetField(units_per_HB_word, THREEFPGAREGS::W1);
  uint64_t w2 = GetField(units_per_HB_word, THREEFPGAREGS::W2);

  SendToEP(0, bdpars::FPGARegEP::TM_PC_SEND_HB_UP_EVERY0, {w0}); // XXX core id?
  SendToEP(0, bdpars::FPGARegEP::TM_PC_SEND_HB_UP_EVERY1, {w1}); // XXX core id?
  SendToEP(0, bdpars::FPGARegEP::TM_PC_SEND_HB_UP_EVERY2, {w2}); // XXX core id?
  Flush();
}

void Driver::ResetBD() {
  // XXX this is only guaranteed to work after bring-up.
  // There's no simple way to enforce this timing if the downstream traffic flow is blocked.
  for (unsigned int i = 0; i < bd_pars_->NumCores; i++) {

    BDWord pReset_1_sReset_1 = PackWord<FPGABDReset>({{FPGABDReset::PRESET, 1}, {FPGABDReset::SRESET, 1}});
    BDWord pReset_0_sReset_1 = PackWord<FPGABDReset>({{FPGABDReset::PRESET, 0}, {FPGABDReset::SRESET, 1}});
    BDWord pReset_0_sReset_0 = PackWord<FPGABDReset>({{FPGABDReset::PRESET, 0}, {FPGABDReset::SRESET, 0}});

    unsigned int delay_us = 500000; // hold reset states for 5 ms (probably conservative)

    SendToEP(i, bdpars::FPGARegEP::BD_RESET, {pReset_1_sReset_1});
    Flush();

    std::this_thread::sleep_for(std::chrono::microseconds(delay_us));

    SendToEP(i, bdpars::FPGARegEP::BD_RESET, {pReset_0_sReset_1});
    Flush();

    std::this_thread::sleep_for(std::chrono::microseconds(delay_us));

    SendToEP(i, bdpars::FPGARegEP::BD_RESET, {pReset_0_sReset_0});
    Flush();

    std::this_thread::sleep_for(std::chrono::microseconds(delay_us));
  }
}

void Driver::IssuePushWords() {
  // we need to send something that will elicit an output, PAT read is the simplest
  // thing we can request

  for (unsigned int i = 0; i < bd_pars_->NumCores; i++) {
    DumpMemSend(i, bdpars::BDMemId::PAT, 0, 2); // we're always two outputs behind
  }

  //// XXX try using writes instead, theory is that there's input slack, not output slack
  //for (unsigned int i = 0; i < bd_pars_->NumCores; i++) {
  //  const std::vector<BDWord> * curr_entries = bd_state_[i].GetMem(bdpars::BDMemId::PAT);
  //  std::vector<BDWord> last_two;
  //  const unsigned int PAT_size = bd_pars_->mem_info_.at(bdpars::BDMemId::PAT).size;
  //  last_two.push_back(curr_entries->at(PAT_size-2));
  //  last_two.push_back(curr_entries->at(PAT_size-1));
  //  SetMem(i, bdpars::BDMemId::PAT, last_two, PAT_size-2);
  //}

  num_pushs_pending_ += 2;
  // we have to do something special when dumping the PAT to skip the push words
}

void Driver::ResetFPGATime() {
  base_time_ = std::chrono::high_resolution_clock::now(); // set time basis

  BDWord reset_time_1 = PackWord<FPGAResetClock>({{FPGAResetClock::RESET_STATE, 1}});
  BDWord reset_time_0 = PackWord<FPGAResetClock>({{FPGAResetClock::RESET_STATE, 0}});
  SendToEP(0, bdpars::FPGARegEP::TM_PC_RESET_TIME, {reset_time_1, reset_time_0}); // XXX core_id?
}

BDTime Driver::GetFPGATime() {
  // the Decoder already decoded the times, ignore the payload and just use the last timestamp
  std::vector<BDTime> times = RecvFromEP(0, bdpars::FPGAOutputEP::UPSTREAM_HB_MSB, 1000).second;
  auto dont_care = RecvFromEP(0, bdpars::FPGAOutputEP::UPSTREAM_HB_LSB, 1000).second; // drain the LSBs too so the queue doesn't pile up
  if (times.size() > 0) {
    return times.back();
  } else {
#ifndef __OPTIMIZE__
    cout << "WARNING: GetFPGATime: haven't received any upstream HBs! Returning 0" << endl;
#endif
    return 0;
  }
}

BDTime Driver::GetDriverTime() const {
  auto time_point_now = std::chrono::high_resolution_clock::now();
  auto ns_now = std::chrono::duration_cast<std::chrono::nanoseconds>(time_point_now - base_time_);
  return ns_now.count();
}

void Driver::SetOKBitFile(std::string bitfile) {
    ok_pars_.ok_bitfile = bitfile;
}

void Driver::InitDAC(unsigned int core_id, bool flush) {
  // List of DAC
  std::array<bdpars::BDHornEP, 12> dac_list {
    bdpars::BDHornEP::DAC_ADC_BIAS_1,
    bdpars::BDHornEP::DAC_ADC_BIAS_2,
    bdpars::BDHornEP::DAC_DIFF_G,
    bdpars::BDHornEP::DAC_DIFF_R,
    bdpars::BDHornEP::DAC_SOMA_OFFSET,
    bdpars::BDHornEP::DAC_SOMA_REF,
    bdpars::BDHornEP::DAC_SYN_EXC,
    bdpars::BDHornEP::DAC_SYN_DC,
    bdpars::BDHornEP::DAC_SYN_INH,
    bdpars::BDHornEP::DAC_SYN_LK,
    bdpars::BDHornEP::DAC_SYN_PD,
    bdpars::BDHornEP::DAC_SYN_PU
  };

  // Set them to default values defined in BDPars
  for(auto& dac_id: dac_list){
    unsigned int dac_count = GetDACDefaultCount(dac_id);
    SetDACCount(core_id, dac_id, dac_count, false);
  }
  if (flush) Flush();
}

void Driver::InitFPGA() {

  cout << "InitFPGA: initializing SGs" << endl;
  for (unsigned int i = 0; i < bd_pars_->NumCores; i++) {
    InitSGEn(i);
    SendSGEns(i, 0);
  }

  cout << "InitFPGA: initializing SFs" << endl;

}

void Driver::InitBD() {

  // BD hard reset
  cout << "InitBD: BD reset cycle" << endl;
  ResetBD();

  for (unsigned int i = 0; i < bd_pars_->NumCores; i++) {
    // turn off traffic
    cout << "InitBD: disabling traffic flow" << endl;
    SetTagTrafficState(i, false);
    SetSpikeTrafficState(i, false);

    // Set the memory delays
    for(auto& mem : {bdpars::BDMemId::AM, bdpars::BDMemId::MM, bdpars::BDMemId::FIFO_PG, bdpars::BDMemId::FIFO_DCT, bdpars::BDMemId::TAT0, bdpars::BDMemId::TAT1, bdpars::BDMemId::PAT}) {
      const unsigned delay_val = 0;
      SetMemoryDelay(i, mem, delay_val, delay_val);
    }

    // init the FIFO
    cout << "InitBD: initializing FIFO" << endl;
    InitFIFO(i);

    cout << "InitBD: programming memories to default values" << endl;
    // initialize memories to sane values (critically, that can't cause infinite loops)
    SetMem(i , bdpars::BDMemId::PAT  , GetDefaultPATEntries()  , 0);
    SetMem(i , bdpars::BDMemId::TAT0 , GetDefaultTAT0Entries() , 0);
    SetMem(i , bdpars::BDMemId::TAT1 , GetDefaultTAT1Entries() , 0);
    SetMem(i , bdpars::BDMemId::MM   , GetDefaultMMEntries()   , 0);
    SetMem(i , bdpars::BDMemId::AM   , GetDefaultAMEntries()   , 0);

    // Initialize neurons
    cout << "InitBD: setting default DAC settings" << endl;
    InitDAC(i, false);

    cout << "InitBD: setting default neuron twiddle bits" << endl;
    // Disable all Somas
    for(unsigned int idx = 0; idx < 4096; ++idx){
      DisableSoma(i, idx);
      SetSomaGain(i, idx, bdpars::SomaGainId::ONE);
      SetSomaOffsetSign(i, idx, bdpars::SomaOffsetSignId::POSITIVE);
      SetSomaOffsetMultiplier(i, idx, bdpars::SomaOffsetMultiplierId::ZERO);
    }

    // Disable all Synapses
    for(unsigned int idx = 0; idx < 1024; ++idx){
      DisableSynapse(i, idx);
      DisableSynapseADC(i, idx);
    }
    // Open all Diffusor cuts
    for(unsigned int idx = 0; idx < 256; ++idx){
      OpenDiffusorAllCuts(i, idx);
    }

    // XXX other stuff to do?
    Flush();
  }
}

void Driver::ClearOutputs() {
  std::vector<uint8_t> up_eps = bd_pars_->GetUpEPs();
  for (auto& it : up_eps) {
    dec_bufs_out_.at(it)->PopAll();
  }
}

void Driver::InitFIFO(unsigned int core_id) {

  PauseTraffic(core_id);

  // turn post-FIFO dumps on, in case you want to watch
  //SetToggle(core_id , bdpars::BDHornEP::TOGGLE_POST_FIFO0 , false, true);
  //SetToggle(core_id , bdpars::BDHornEP::TOGGLE_POST_FIFO1 , false, true);
  //cout << "configured FIFO valves for dump" << endl;

  // make FIFO_HT head = tail (doesn't matter what you send)
  SendToEP(core_id, bdpars::BDHornEP::INIT_FIFO_HT, {0});
  Flush();


  // send all tag values to DCT FIF0 to dirty them so they flush
  std::vector<BDWord> all_tag_vals;
  for (unsigned int i = 0; i < bd_pars_->mem_info_.at(bdpars::BDMemId::FIFO_DCT).size; i++) {
    all_tag_vals.push_back(PackWord<FIFOInputTag>({{FIFOInputTag::TAG, i}}));
  }
  SendToEP(core_id, bdpars::BDHornEP::INIT_FIFO_DCT, all_tag_vals);
  Flush();

  // resume traffic will wait for the traffic drain timer before turning traffic regs back on
  std::this_thread::sleep_for(std::chrono::microseconds(1000000));

  ResumeTraffic(core_id);

  // turn traffic on, dumps off
  //SetToggle(core_id , bdpars::BDHornEP::TOGGLE_POST_FIFO0 , true, true);
  //SetToggle(core_id , bdpars::BDHornEP::TOGGLE_POST_FIFO1 , true, true);
  //cout << "fifo valves configured to pass traffic"  << endl;
}


int Driver::Start() {
  // start all worker threads
  enc_->Start();
  dec_->Start();
  cout << "enc and dec started" << endl;

  int comm_state = 0;

#ifdef BD_COMM_TYPE_OPALKELLY
  // Initialize Opal Kelly Board
  comm_state = static_cast<comm::CommOK*>(comm_)->Init(ok_pars_.ok_bitfile, ok_pars_.ok_serial);
#endif

  if (comm_state >= 0) {
    comm_->StartStreaming();
  }

  return comm_state;
}

void Driver::Stop() {
  enc_->Stop();
  dec_->Stop();
  comm_->StopStreaming();
}

void Driver::Flush() {

    // Call phantom DAC to push two other words into BD to
    // circumvent the synchronizer bug
    BDWord word = PackWord<DACWord>({{DACWord::DAC_VALUE, 1}, {DACWord::DAC_TO_ADC_CONN, 0}});
    for(unsigned int _core_id = 0; _core_id < bd_pars_->NumCores; _core_id ++){
        SetBDRegister(_core_id, bdpars::BDHornEP::DAC_UNUSED, word, false);
        SetBDRegister(_core_id, bdpars::BDHornEP::DAC_UNUSED, word, false);
    }

  // pushes a special ep_code

  // XXX note that the order that the user makes timed vs sequenced downstream calls is lost!
  //
  // If I call:
  // SendSpikes(), SetMem(), SendSpikes(), Flush()
  // there's no guarantee that the memory is going to be set after the first batch of spikes is sent
  //
  // If you really want that, do this:
  // SendSpikes(), Flush(),
  // SetMem(), Flush(),
  // SendSpikes(), Flush()
  //
  // The order of sequenced calls is conserved, and, of course, the timing is correct
  //
  // If I call:
  // SendSpikes(a), SetMem(x), SendSpikes(b), SetMem(y), Flush()
  //
  // then SetMem(x) is guaranteed to occur before SetMem(y)
  // and the traffic of SendSpikes(a) is interleaved with SendSpikes(b) as necessary

  unsigned int num_words = 0;
  num_words += timed_queue_.size();

  // send the timed traffic first (it's more important, I guess)
  auto from_queue = std::make_unique<std::vector<EncInput>>();
  from_queue->swap(timed_queue_);
  enc_buf_in_->Push(std::move(from_queue));

  // then send the sequenced traffic
  while (!sequenced_queue_.empty()) {
    num_words += sequenced_queue_.front()->size();
    enc_buf_in_->Push(std::move(sequenced_queue_.front()));
    sequenced_queue_.pop();
  }

  EncInput flush;
  flush.FPGA_ep_code = EncInput::kFlushCode;
  flush.core_id = 0; // don't care about the other fields
  flush.payload = 0;
  flush.time = 0;

  auto flush_vect = std::make_unique<std::vector<EncInput>>(1, flush);
  enc_buf_in_->Push(std::move(flush_vect));

}

/// Set toggle traffic_en only, keep dump_en the same, returns previous traffic_en.
/// If register state has not been set, dump_en -> 0
bool Driver::SetToggleTraffic(unsigned int core_id, bdpars::BDHornEP reg_id, bool en, bool flush) {
  bool traffic_en, dump_en, reg_valid;
  std::tie(traffic_en, dump_en, reg_valid) = bd_state_[core_id].GetToggle(reg_id);
  SetToggle(core_id, reg_id, en, dump_en & reg_valid, flush);
  return traffic_en;
}

/// Set toggle dump_en only, keep traffic_en the same, returns previous dump_en.
/// If register state has not been set, traffic_en -> 0
bool Driver::SetToggleDump(unsigned int core_id, bdpars::BDHornEP reg_id, bool en, bool flush) {
  bool traffic_en, dump_en, reg_valid;
  std::tie(traffic_en, dump_en, reg_valid) = bd_state_[core_id].GetToggle(reg_id);
  SetToggle(core_id, reg_id, traffic_en & reg_valid, en, flush);
  return dump_en;
}

/// Turn on tag traffic in datapath (also calls Start/KillSpikes)
void Driver::SetTagTrafficState(unsigned int core_id, bool en, bool flush) {

  const std::vector<bdpars::BDHornEP> kTagRegs = {
      bdpars::BDHornEP::TOGGLE_PRE_FIFO,
      bdpars::BDHornEP::TOGGLE_POST_FIFO0,
      bdpars::BDHornEP::TOGGLE_POST_FIFO1};

  for (auto& it : kTagRegs) {
    SetToggleTraffic(core_id, it, en, false);
  }
  if (flush) Flush();
}

/// Turn on spike outputs for all neurons
void Driver::SetSpikeTrafficState(unsigned int core_id, bool en, bool flush) {
  SetToggleTraffic(core_id, bdpars::BDHornEP::NEURON_DUMP_TOGGLE, en, flush);
}

/// Turn on spike outputs for all neurons
void Driver::SetSpikeDumpState(unsigned int core_id, bool en, bool flush) {
  SetToggleDump(core_id, bdpars::BDHornEP::NEURON_DUMP_TOGGLE, en, flush);
}

void Driver::PauseTraffic(unsigned int core_id) {
  assert(last_traffic_state_[core_id].size() == 0 && "called PauseTraffic twice before calling ResumeTraffic");
  last_traffic_state_[core_id] = {};
  for (auto& reg_id : kTrafficRegs) {
    bool last_state = SetToggleTraffic(core_id, reg_id, false);
    last_traffic_state_[core_id].push_back(last_state);
  }
  Flush();
  bd_state_[core_id].WaitForTrafficOff();
}

void Driver::ResumeTraffic(unsigned int core_id) {
  assert(last_traffic_state_[core_id].size() > 0 && "called ResumeTraffic before calling PauseTraffic");
  unsigned int i = 0;
  for (auto& reg_id : kTrafficRegs) {
    SetToggleTraffic(core_id, reg_id, last_traffic_state_[core_id][i]);
    i++;
  }
  last_traffic_state_[core_id] = {};
  Flush();
}


void Driver::SetMemoryDelay(unsigned int core_id, bdpars::BDMemId mem_id, unsigned int read_value, unsigned int write_value, bool flush) {
  BDWord word = PackWord<DelayWord>({{DelayWord::READ_DELAY, read_value},
                                           {DelayWord::WRITE_DELAY, write_value}});
  SetBDRegister(core_id, bd_pars_->mem_info_.at(mem_id).delay_reg, word, flush);
}

std::vector<BDWord> Driver::GetPreFIFODump(unsigned int core_id) {
  return RecvFromEP(core_id, bdpars::BDFunnelEP::DUMP_PRE_FIFO, 1000).first;
}

std::pair<std::vector<BDWord>, std::vector<BDWord> > Driver::GetPostFIFODump(unsigned int core_id) {
  std::vector<BDWord> tags0, tags1;

  tags0 = RecvFromEP(core_id, bdpars::BDFunnelEP::DUMP_POST_FIFO0, 1000).first;
  tags1 = RecvFromEP(core_id, bdpars::BDFunnelEP::DUMP_POST_FIFO1, 1000).first;

  return std::make_pair(tags0, tags1);
}

std::pair<unsigned int, unsigned int> Driver::GetFIFOOverflowCounts(unsigned int core_id) {
  std::vector<BDWord> ovflw0 = RecvFromEP(core_id, bdpars::BDFunnelEP::OVFLW0, 1000).first;
  std::vector<BDWord> ovflw1 = RecvFromEP(core_id, bdpars::BDFunnelEP::OVFLW1, 1000).first;
  return {ovflw0.size(), ovflw1.size()};
}

void Driver::SetDACCount(unsigned int core_id, bdpars::BDHornEP signal_id, unsigned int value, bool flush) {

  // look up state of connection to ADC
  BDWord reg_val = bd_state_.at(core_id).GetReg(signal_id).first;
  bool DAC_to_ADC_conn_curr_state = GetField(reg_val, DACWord::DAC_TO_ADC_CONN);

  BDWord word = PackWord<DACWord>({{DACWord::DAC_VALUE, value - 1}, {DACWord::DAC_TO_ADC_CONN, DAC_to_ADC_conn_curr_state}});
  //SetBDRegister(core_id, signal_id, word, flush);
  SetBDRegister(core_id, signal_id, word, false);
  SetBDRegister(core_id, bdpars::BDHornEP::DAC_UNUSED, word, false);
  SetBDRegister(core_id, bdpars::BDHornEP::DAC_UNUSED, word, flush);
}

void Driver::SetDACValue(unsigned int core_id, bdpars::BDHornEP signal_id, float value, bool flush) {
    auto _dac = bd_pars_->dac_info_.at(signal_id);
    unsigned int dac_count;
    if(value < 0){
        dac_count = _dac.default_count;
    }else{
        dac_count = static_cast<unsigned int>(value / bdpars::DACInfo::DAC_UNIT_CURRENT * _dac.scaling);
    }
    SetDACCount(core_id, signal_id, dac_count, flush);
}

unsigned int Driver::GetDACCurrentCount(unsigned int core_id, bdpars::BDHornEP signal_id){
  BDWord reg_val =  GetRegState(core_id, signal_id).first;
  return GetField(reg_val, DACWord::DAC_VALUE) + 1;
}

  unsigned int Driver::GetDACScaling(bdpars::BDHornEP signal_id){ return bd_pars_->dac_info_.at(signal_id).scaling; }

  unsigned int Driver::GetDACDefaultCount(bdpars::BDHornEP signal_id){ return bd_pars_->dac_info_.at(signal_id).default_count; }

  float Driver::GetDACUnitCurrent(bdpars::BDHornEP signal_id){ return bdpars::DACInfo::DAC_UNIT_CURRENT / static_cast<float>(bd_pars_->dac_info_.at(signal_id).scaling); }

void Driver::SetDACtoADCConnectionState(unsigned int core_id, bdpars::BDHornEP signal_id, bool en, bool flush) {

  // look up DAC value
  BDWord reg_val = bd_state_.at(core_id).GetReg(signal_id).first;
  uint64_t DAC_curr_val = GetField(reg_val, DACWord::DAC_VALUE);

  BDWord word = PackWord<DACWord>({{DACWord::DAC_VALUE, DAC_curr_val}, {DACWord::DAC_TO_ADC_CONN, en}});
  SetBDRegister(core_id, signal_id, word, flush);
}

/// Set large/small current scale for either ADC
void Driver::SetADCScale(unsigned int core_id, unsigned int adc_id, const std::string& small_or_large) {
  bool small = small_or_large.compare("small");
  bool large = small_or_large.compare("large");
  if (!small and !large) assert(false && "<small_or_large> must be \"small\" or \"large\"");
  
  BDWord curr_state = bd_state_.at(core_id).GetReg(bdpars::BDHornEP::ADC).first;
  unsigned int curr_small_large_0 = GetField(curr_state, ADCWord::ADC_SMALL_LARGE_CURRENT_0);
  unsigned int curr_small_large_1 = GetField(curr_state, ADCWord::ADC_SMALL_LARGE_CURRENT_1);
  unsigned int curr_enable        = GetField(curr_state, ADCWord::ADC_OUTPUT_ENABLE);

  assert((adc_id == 0 || adc_id == 1) && "<adc_id> must be 0 or 1");
  BDWord word_to_prog;
  unsigned int small_large_bit = static_cast<unsigned int>(large);
  if (adc_id == 0) {
      word_to_prog = PackWord<ADCWord>({{ADCWord::ADC_SMALL_LARGE_CURRENT_0, small_large_bit},
                                        {ADCWord::ADC_SMALL_LARGE_CURRENT_1, curr_small_large_1},
                                        {ADCWord::ADC_OUTPUT_ENABLE,         curr_enable}});
  } else {
      word_to_prog = PackWord<ADCWord>({{ADCWord::ADC_SMALL_LARGE_CURRENT_0, curr_small_large_0},
                                        {ADCWord::ADC_SMALL_LARGE_CURRENT_1, small_large_bit},
                                        {ADCWord::ADC_OUTPUT_ENABLE,         curr_enable}});
  }
  SetBDRegister(core_id, bdpars::BDHornEP::ADC, word_to_prog);
}

/// Turn ADC output on
void Driver::SetADCTrafficState(unsigned int core_id, bool en) {
  BDWord curr_state = bd_state_.at(core_id).GetReg(bdpars::BDHornEP::ADC).first;
  unsigned int curr_small_large_0 = GetField(curr_state, ADCWord::ADC_SMALL_LARGE_CURRENT_0);
  unsigned int curr_small_large_1 = GetField(curr_state, ADCWord::ADC_SMALL_LARGE_CURRENT_1);

  BDWord word_to_prog = PackWord<ADCWord>({{ADCWord::ADC_SMALL_LARGE_CURRENT_0, curr_small_large_0},
                                           {ADCWord::ADC_SMALL_LARGE_CURRENT_1, curr_small_large_1},
                                           {ADCWord::ADC_OUTPUT_ENABLE, en}});

  SetBDRegister(core_id, bdpars::BDHornEP::ADC, word_to_prog);
}

////////////////////////////////////////////////////////////////////////////////
// Neuron controls
////////////////////////////////////////////////////////////////////////////////
///
/// Soma has an ID between 0 and 4095. Synapse has an ID between 0 and 1023.
///
/// The Soma ID is split as follows:
/// Tile ID: 8 bits     => 0 - 255
/// In-tile ID: 4 bits  => 0 - 15
///
/// The Synapse ID is split as follows:
/// Tile ID: 8 bits     => 0 - 255
/// In-tile ID: 2 bits  => 0 - 3
template<class U>
    void Driver::SetConfigMemory(unsigned int core_id, unsigned int elem_id,
                       std::unordered_map<U, std::vector<unsigned int>> config_map,
                       U config_type,
                       bool config_value) {
    unsigned int num_per_tile = config_map[config_type].size();
    unsigned int tile_id = elem_id / num_per_tile;
    unsigned int intra_tile_id = elem_id % num_per_tile;
    unsigned int tile_mem_loc = config_map[config_type][intra_tile_id];
    unsigned int row        = tile_mem_loc % 8;
    unsigned int column     = tile_mem_loc / 16;
    unsigned int bit_select = (tile_mem_loc % 16) / 8;
    std::vector<BDWord> config_word {PackWord<NeuronConfig>({
      {NeuronConfig::ROW_HI, (row >> 1) & 0x03},
      {NeuronConfig::ROW_LO, row & 0x01},
      {NeuronConfig::COL_HI, (column >> 2) & 0x01},
      {NeuronConfig::COL_LO, column & 0x03},
      {NeuronConfig::BIT_SEL, bit_select & 0x01},
      {NeuronConfig::BIT_VAL, config_value},
      {NeuronConfig::TILE_ADDR, tile_id}
    })};

    bd_state_[core_id].SetNeuronConfigMem(core_id, tile_id, intra_tile_id, config_type, config_value);

    //PauseTraffic(core_id);
    SendToEP(core_id, bdpars::BDHornEP::NEURON_CONFIG, config_word);
    //ResumeTraffic(core_id);
  }

////////////////////////////////////////////////////////////////////////////////
// Soma controls
////////////////////////////////////////////////////////////////////////////////

void Driver::SetSomaEnableStatus(unsigned int core_id, unsigned int soma_id,
                                 bdpars::SomaStatusId status) {
    bool bit_val = (status == bdpars::SomaStatusId::DISABLED ? 0 : 1);
    SetSomaConfigMemory(core_id, soma_id, bdpars::ConfigSomaID::ENABLE, bit_val);
}

void Driver::SetSomaGain(unsigned int core_id, unsigned int soma_id,
                         bdpars::SomaGainId gain) {
    bool g1, g0;
    switch (gain) {
        case bdpars::SomaGainId::ONE_FOURTH :
            g1 = 0; g0 = 0;
            break;
        case bdpars::SomaGainId::ONE_THIRD :
            g1 = 0; g0 = 1;
            break;
        case bdpars::SomaGainId::ONE_HALF :
            g1 = 1; g0 = 0;
            break;
        case bdpars::SomaGainId::ONE :
            g1 = 1; g0 = 1;
            break;
        default:
            break;
    }
    SetSomaConfigMemory(core_id, soma_id, bdpars::ConfigSomaID::GAIN_0, g0);
    SetSomaConfigMemory(core_id, soma_id, bdpars::ConfigSomaID::GAIN_1, g1);
}

void Driver::SetSomaOffsetSign(unsigned int core_id, unsigned int soma_id,
                               bdpars::SomaOffsetSignId offset_sign) {
    bool bit_val = (offset_sign == bdpars::SomaOffsetSignId::POSITIVE ? 0 : 1);
    SetSomaConfigMemory(core_id, soma_id, bdpars::ConfigSomaID::SUBTRACT_OFFSET, bit_val);
}

void Driver::SetSomaOffsetMultiplier(unsigned int core_id, unsigned int soma_id,
                                     bdpars::SomaOffsetMultiplierId offset_multiplier) {
    bool b1, b0;
    switch (offset_multiplier) {
        case bdpars::SomaOffsetMultiplierId::ZERO :
            b1 = 0; b0 = 0;
            break;
        case bdpars::SomaOffsetMultiplierId::ONE :
            b1 = 0; b0 = 1;
            break;
        case bdpars::SomaOffsetMultiplierId::TWO :
            b1 = 1; b0 = 0;
            break;
        case bdpars::SomaOffsetMultiplierId::THREE :
            b1 = 1; b0 = 1;
            break;
        default:
            break;
    }
    SetSomaConfigMemory(core_id, soma_id, bdpars::ConfigSomaID::OFFSET_0, b0);
    SetSomaConfigMemory(core_id, soma_id, bdpars::ConfigSomaID::OFFSET_1, b1);
}

////////////////////////////////////////////////////////////////////////////////
// Synapse controls
////////////////////////////////////////////////////////////////////////////////
void Driver::SetSynapseEnableStatus(unsigned int core_id, unsigned int synapse_id,
                                    bdpars::SynapseStatusId status) {
    bool bit_val = (status == bdpars::SynapseStatusId::ENABLED ? 0 : 1);
    SetSynapseConfigMemory(core_id, synapse_id, bdpars::ConfigSynapseID::SYN_DISABLE, bit_val);
}

void Driver::SetSynapseADCStatus(unsigned int core_id, unsigned int synapse_id,
                                 bdpars::SynapseStatusId status) {
    bool bit_val = (status == bdpars::SynapseStatusId::ENABLED ? 0 : 1);
    SetSynapseConfigMemory(core_id, synapse_id, bdpars::ConfigSynapseID::ADC_DISABLE, bit_val);
}

////////////////////////////////////////////////////////////////////////////////
// Diffusor controls
////////////////////////////////////////////////////////////////////////////////
void Driver::SetDiffusorCutStatus(unsigned int core_id,
                                  unsigned int tile_id,
                                  bdpars::DiffusorCutLocationId cut_id,
                                  bdpars::DiffusorCutStatusId status) {
    bool bit_val = (status == bdpars::DiffusorCutStatusId::CLOSE ? 0 : 1);
    SetDiffusorConfigMemory(core_id, tile_id, cut_id, bit_val);
}

void Driver::SetDiffusorAllCutsStatus(unsigned int core_id,
                                      unsigned int tile_id,
                                      bdpars::DiffusorCutStatusId status) {
    bool bit_val = (status == bdpars::DiffusorCutStatusId::CLOSE ? 0 : 1);

    SetDiffusorConfigMemory(core_id, tile_id, bdpars::DiffusorCutLocationId::NORTH_LEFT, bit_val);
    SetDiffusorConfigMemory(core_id, tile_id, bdpars::DiffusorCutLocationId::NORTH_RIGHT, bit_val);
    SetDiffusorConfigMemory(core_id, tile_id, bdpars::DiffusorCutLocationId::WEST_TOP, bit_val);
    SetDiffusorConfigMemory(core_id, tile_id, bdpars::DiffusorCutLocationId::WEST_BOTTOM, bit_val);
}

void Driver::SetMem(
    unsigned int core_id,
    bdpars::BDMemId mem_id,
    const std::vector<BDWord> &data,
    unsigned int start_addr) {

  // update BDState
  bd_state_.at(core_id).SetMem(mem_id, start_addr, data);

  // depending on which memory this is, encapsulate differently
  std::vector<BDWord> encapsulated_words;
  if (mem_id == bdpars::BDMemId::PAT) {
    encapsulated_words = PackRWProgWords<PATWrite>(data, start_addr);
  } else if (mem_id == bdpars::BDMemId::TAT0 || mem_id == bdpars::BDMemId::TAT1) {
    encapsulated_words = PackRIWIProgWords<TATSetAddress, TATWriteIncrement>(data, start_addr);
  } else if (mem_id == bdpars::BDMemId::MM) {
    encapsulated_words = PackRIWIProgWords<MMSetAddress, MMWriteIncrement>(data, start_addr);
  } else if (mem_id == bdpars::BDMemId::AM) {
    encapsulated_words = PackRMWProgWords<AMSetAddress, AMReadWrite, AMIncrement>(data, start_addr);
  } else {
    assert(false && "Bad memory ID");
  }

  // if it's an AM or MM word, need further encapsulation
  if (mem_id == bdpars::BDMemId::MM) {
    encapsulated_words = PackAMMMWord<MMEncapsulation>(encapsulated_words);
  } else if (mem_id == bdpars::BDMemId::AM) {
    encapsulated_words = PackAMMMWord<AMEncapsulation>(encapsulated_words);
  }

  // transmit to horn
  PauseTraffic(core_id);
  bdpars::BDHornEP horn_ep = bd_pars_->mem_info_.at(mem_id).prog_leaf;
  SendToEP(core_id, horn_ep, encapsulated_words);
  ResumeTraffic(core_id);

  Flush();

  if (mem_id == bdpars::BDMemId::AM) { // if we're programming the AM, we're also dumping the AM, need to sink what comes back
    bdpars::BDFunnelEP funnel_ep = bd_pars_->mem_info_.at(mem_id).dump_leaf;

    // pop out the last two words
    IssuePushWords();

    double timeout_s = 2; // keep reading for 2s
    auto start = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = now - start;

    unsigned int n_recvd = 0;
    while (n_recvd < data.size() && diff.count() < timeout_s) {
      std::pair<std::vector<BDWord>, std::vector<BDTime>> recvd = RecvFromEP(core_id, funnel_ep, 10000);
      n_recvd += recvd.first.size();
      now = std::chrono::high_resolution_clock::now();
      diff = now - start;
    }
    if (diff.count() > timeout_s) {
      cout << "WARNING! while programming AM, got fewer words than we expected" << endl;
      cout << "  got " << n_recvd << " vs " << data.size() << endl;
    }
    if (n_recvd > data.size()) {
      cout << "WARNING! while programming AM, got more words than we expected" << endl;
      cout << "  got " << n_recvd << " vs " << data.size() << endl;
    }
  }
}

/// helper for DumpMem
void Driver::DumpMemSend(unsigned int core_id, bdpars::BDMemId mem_id, unsigned int start_addr, unsigned int end_addr) {
  // make dump words

  assert(start_addr >= 0);
  assert(end_addr <= bd_pars_->mem_info_.at(mem_id).size);

  std::vector<BDWord> encapsulated_words;
  if (mem_id == bdpars::BDMemId::PAT) {
    encapsulated_words = PackRWDumpWords<PATRead>(start_addr, end_addr);
  } else if (mem_id == bdpars::BDMemId::TAT0 || mem_id == bdpars::BDMemId::TAT1) {
    encapsulated_words = PackRIWIDumpWords<TATSetAddress, TATReadIncrement>(start_addr, end_addr);
  } else if (mem_id == bdpars::BDMemId::MM) {
    encapsulated_words = PackRIWIDumpWords<MMSetAddress, MMReadIncrement>(start_addr, end_addr);
  } else if (mem_id == bdpars::BDMemId::AM) {
    // a little tricky, reprogramming is the same as dump
    // need to write back whatever is currently in memory
    const std::vector<BDWord> *curr_data = bd_state_.at(core_id).GetMem(bdpars::BDMemId::AM);
    std::vector<BDWord> to_rewrite;
    for (unsigned int i = start_addr; i < end_addr; i++) {
      to_rewrite.push_back(curr_data->at(i));
    }
    encapsulated_words = PackRMWProgWords<AMSetAddress, AMReadWrite, AMIncrement>(to_rewrite, start_addr);
  } else {
    assert(false && "Bad memory ID");
  }

  // if it's an AM or MM word, need further encapsulation
  if (mem_id == bdpars::BDMemId::MM) {
    encapsulated_words = PackAMMMWord<MMEncapsulation>(encapsulated_words);
  } else if (mem_id == bdpars::BDMemId::AM) {
    encapsulated_words = PackAMMMWord<AMEncapsulation>(encapsulated_words);
  }

  // transmit read words, then block until all dump words have been received
  // XXX if something goes terribly wrong and not all the words come back, this will hang
  bdpars::BDHornEP horn_ep = bd_pars_->mem_info_.at(mem_id).prog_leaf;

  PauseTraffic(core_id);

  SendToEP(core_id, horn_ep, encapsulated_words);

  Flush();

  ResumeTraffic(core_id);
}

std::vector<BDWord> Driver::DumpMemRecv(unsigned int core_id, bdpars::BDMemId mem_id, unsigned int dump_first_n, unsigned int wait_for_us) {

  // sleep to wait for the outputs to come back
  std::this_thread::sleep_for(std::chrono::microseconds(wait_for_us));

  bdpars::BDFunnelEP funnel_ep = bd_pars_->mem_info_.at(mem_id).dump_leaf;

  std::pair<std::vector<BDWord>, std::vector<BDTime>> recvd = RecvFromEP(core_id, funnel_ep, 1); // timeout immediately if nothing is there
  std::vector<BDWord>& payloads = recvd.first;

  if (payloads.size() == 0) {
    cout << "WARNING: DumpMemRecv timed out! Expected output from memory" << endl;
    return payloads;
  }
  cout << "payload size: " << payloads.size() << endl;

  // if this is the PAT, need to chop off the first num_pushs_pending_ - 2 outputs
  // (the last two pending we just put on, come after the memory words)
  if (mem_id == bdpars::BDMemId::PAT) {
    std::vector<BDWord> payloads_tmp;
    payloads_tmp.swap(payloads);
    unsigned int pushs_absorbed = 0;
    for (unsigned int i = 0; i < payloads_tmp.size(); i++) {
      if (i >= num_pushs_pending_ - 2) {
        payloads.push_back(payloads_tmp.at(i));
      } else {
        pushs_absorbed += 1;
      }
    }
    num_pushs_pending_ -= pushs_absorbed;
    //cout << "possibly discarding initial PAT words" << endl;
    //cout << payloads_tmp.size() << " -> " << payloads.size() << endl;
  }

  //cout << "num pushs pending before: " << num_pushs_pending_ << endl;

  // we might have received more words than we expected
  // if this is the PAT, and there are <2, they're probably just the pushes we just sent
  // (they can get pushed out by other traffic)
  // otherwise, something weird happened
  if (payloads.size() > dump_first_n) {
    unsigned int extra_words_after = payloads.size() - dump_first_n;
    if (mem_id == bdpars::BDMemId::PAT) {
      if (extra_words_after > num_pushs_pending_) {
        cout << "WARNING! Got more words from PAT memory than expected" << endl;
        num_pushs_pending_ = 0; // best guess of what to do
      } else {
        num_pushs_pending_ -= extra_words_after;
      }
    } else {
      if (extra_words_after > 0) {
        cout << "WARNING! Got more words from non-PAT memory than expected" << endl;
        cout << "  expected: " << dump_first_n << endl;
        cout << "  received: " << payloads.size() << endl;
      }
    }
  } else if (dump_first_n > payloads.size()) {
    cout << "WARNING! didn't get the full dump size we requested" << endl;
  }

  //cout << "num pushs pending after: " << num_pushs_pending_ << endl;

  return payloads;
}

std::vector<BDWord> Driver::DumpMemRange(unsigned int core_id, bdpars::BDMemId mem_id, unsigned int start, unsigned int end) {

  assert(end > start);

  DumpMemSend(core_id, mem_id, start, end);

  // issue an additional two PAT reads to push out the last two words
  IssuePushWords();

  auto to_return = DumpMemRecv(core_id, mem_id, end - start, 2000000); // wait 2s

  return to_return;

}

std::vector<BDWord> Driver::DumpMem(unsigned int core_id, bdpars::BDMemId mem_id) {

  unsigned int mem_size = bd_pars_->mem_info_.at(mem_id).size;
  return DumpMemRange(core_id, mem_id, 0, mem_size);

}

template <class TWrite>
std::vector<BDWord> Driver::PackRWProgWords(const std::vector<BDWord>& payload, unsigned int start_addr) const
{
  std::vector<BDWord> retval;

  uint64_t addr = static_cast<uint64_t>(start_addr);
  for (auto& it : payload) {
    retval.push_back(PackWord<TWrite>({{TWrite::ADDRESS, addr}, {TWrite::DATA, it}}));
    addr++;
  }

  return retval;
}

template <class TRead>
std::vector<BDWord> Driver::PackRWDumpWords(unsigned int start_addr, unsigned int end_addr) const
{
  std::vector<BDWord> retval;

  uint64_t addr = static_cast<uint64_t>(start_addr);
  for (unsigned int i = 0; i < end_addr - start_addr; i++) {
    retval.push_back(PackWord<TRead>({{TRead::ADDRESS, addr}}));
    addr++;
  }

  return retval;
}

template <class TSet, class TWrite>
std::vector<BDWord> Driver::PackRIWIProgWords(
    const std::vector<BDWord>& payload,
    unsigned int start_addr) const
{
  std::vector<BDWord> retval;

  // address word
  retval.push_back(PackWord<TSet>({{TSet::ADDRESS, start_addr}}));

  for (auto& it : payload) {
    retval.push_back(PackWord<TWrite>({{TWrite::DATA, it}}));
  }

  return retval;
}

template <class TSet, class TRead>
std::vector<BDWord> Driver::PackRIWIDumpWords(
    unsigned int start_addr, unsigned int end_addr) const
{
  std::vector<BDWord> retval;

  // address word
  retval.push_back(PackWord<TSet>({{TSet::ADDRESS, start_addr}}));

  for (unsigned int i = 0; i < end_addr - start_addr; i++) {
    retval.push_back(PackWord<TRead>({}));
  }

  return retval;
}

template <class TSet, class TReadWrite, class TInc>
std::vector<BDWord> Driver::PackRMWProgWords(
    const std::vector<BDWord>& payload,
    unsigned int start_addr) const
{
  // RMWProgWord is the same as RMWDumpWord

  std::vector<BDWord> retval;

  // address word
  uint64_t addr = static_cast<uint64_t>(start_addr);
  retval.push_back(PackWord<TSet>({{TSet::ADDRESS, addr}}));

  for (auto& it : payload) {
    retval.push_back(PackWord<TReadWrite>({{TReadWrite::DATA, it}}));
    retval.push_back(PackWord<TInc>({}));
  }

  return retval;
}

template <class AMorMMEncapsulation>
std::vector<BDWord> Driver::PackAMMMWord(const std::vector<BDWord>& payload_data) const
{
  std::vector<BDWord> retval;
  for (unsigned int i = 0; i < payload_data.size(); i++) {
    uint64_t stop_bit;
    if (i == payload_data.size() - 1) {
      stop_bit = 1;
    } else {
      stop_bit = 0;
    }

    BDWord word = PackWord<AMorMMEncapsulation>(
          {{AMorMMEncapsulation::PAYLOAD, payload_data[i]}, {AMorMMEncapsulation::AMMM_STOP, stop_bit}});
    retval.push_back(word);
  }
  return retval;
}

void Driver::SendSpikes(unsigned int core_id, const std::vector<BDWord>& spikes, const std::vector<BDTime> times, bool flush) {
  assert(spikes.size() == times.size());

  // do something with times
  SendToEP(core_id, bdpars::BDHornEP::NEURON_INJECT, spikes, times);
  if (flush) Flush();
}

std::pair<std::vector<unsigned int>, std::vector<float>> Driver::RecvXYSpikesMasked(unsigned int core_id){
    // Timeout of 1us
    auto spikes = RecvFromEP(core_id, bdpars::BDFunnelEP::NRNI, 1);
    auto aer_addresses = spikes.first;
    auto aer_times = spikes.second;
    std::vector<unsigned int> xy_addresses(4096, 0);
    std::vector<float> xy_times(4096, 0);
    for(unsigned int idx = 0; idx < aer_addresses.size(); ++idx){
        // bug in synchronizer can result in out of bond aer addresses
        // due to the bit-flipping bug
        auto _addr = aer_addresses[idx];
        if(_addr >=0 && _addr < 4096){
            auto _xy = GetSomaXYAddr(aer_addresses[idx]);
            xy_addresses[_xy] = 1;
            xy_times[_xy] = static_cast<float>(aer_times[idx]) * 1e-9;
        }else {
            cout << "WARNING: Invalid spike address: " << _addr << endl;
        }
    }
    return {xy_addresses, xy_times};
}

void Driver::SendTags(unsigned int core_id, const std::vector<BDWord>& tags, const std::vector<BDTime> times, bool flush) {
  assert(times.size() == 0 || tags.size() == times.size());

  // do something with times
  SendToEP(core_id, bdpars::BDHornEP::RI, tags, times);
  if (flush) Flush();
}

void Driver::SendSGEns(unsigned int core_id, BDTime time) {
  // send all SG enables
  assert(max_num_SG_ <= 256); // that's what this was written for
  uint16_t en_word;
  for (unsigned int gen_idx = 0; gen_idx < SG_en_.at(core_id).size(); gen_idx++) {
    unsigned int bit_idx = gen_idx % 16;
    uint16_t bit_sel = 1 << bit_idx;

    if (bit_idx == 0) en_word = 0;

    if (SG_en_[core_id][gen_idx]) {
      en_word |= bit_sel;
    }

    if (bit_idx == 15) {
      bdpars::FPGARegEP SG_reg_ep = bd_pars_->GenIdxToSG_GENS_EN(gen_idx);
      //cout << "enable" << en_word << endl;
      SendToEP(core_id, bd_pars_->DnEPCodeFor(SG_reg_ep), {en_word}, {time});
    }
  }
}

void Driver::SetSpikeGeneratorRates(
    unsigned int core_id,
    const std::vector<unsigned int>& gen_idxs,
    const std::vector<unsigned int>& tags,
    const std::vector<int>& rates,
    BDTime time,
    bool flush) {

  assert(tags.size() == rates.size());

  std::vector<BDWord> SG_prog_words;

  unsigned int units_per_sec = 1e9 / ns_per_unit_;

  // program periods/tag output idxs
  for (unsigned int i = 0; i < tags.size(); i++) {

    unsigned int gen_idx = gen_idxs.at(i);
    unsigned int tag = tags.at(i);

    // (period in time units) = (units/sec) / (rate in 1/sec)
    const unsigned int max_period = (1 << FieldWidth(FPGASGWORD::PERIOD)) - 1;

    unsigned int rate;
    unsigned int sign;
    if(rates.at(i) >= 0) {
      rate = rates.at(i);
      sign = 0;
    } else {
      rate = -rates.at(i);
      sign = 1;
    }

    unsigned int period = rate > 0 ? units_per_sec / rate : max_period;
    period = period >= max_period ? max_period : period; // possible to get a period longer than the max programmable
    //cout << "programming SG " << gen_idx << " to target tag " << tag << " at rate " << rate << " sign " << sign << endl;
    //cout << "  period : " << period << " time units" << endl;
    //cout << "  starting at : " << time << " ns" << endl << endl;

    SG_prog_words.push_back(PackWord<FPGASGWORD>({{FPGASGWORD::TAG, tag}, {FPGASGWORD::PERIOD, period}, {FPGASGWORD::GENIDX, gen_idx}, {FPGASGWORD::SIGN, sign}}));
  }

  //std::vector<BDTime> SG_prog_times(SG_prog_words.size(), time);

  // XXX this is a hack to avoid misordering upon sorting
  // we have the same problem here that the serializer does
  std::vector<BDTime> SG_prog_times;
  for (unsigned int i = 0; i < SG_prog_words.size(); i++) {
    SG_prog_times.push_back(time + UnitsToNs(1));
  }

  SendToEP(core_id, bd_pars_->DnEPCodeFor(bdpars::FPGAChannelEP::SG_PROGRAM_MEM), SG_prog_words, SG_prog_times);

  // update generator enable states
  for (unsigned int i = 0; i < tags.size(); i++) {

    unsigned int gen_idx = gen_idxs.at(i);
    unsigned int rate = rates.at(i);

    bool new_state = rate > 0;
    SG_en_[core_id][gen_idx] = new_state;
  }

  // send all SG enables, regardless if only one was changed
  SendSGEns(core_id, time);

  // set number of SGs used
  int highest_used = GetHighestSGEn(core_id);
  SendToEP(core_id, bd_pars_->DnEPCodeFor(bdpars::FPGARegEP::SG_GENS_USED), {highest_used+1}, {time});
  //cout << "number of generators: " << highest_used + 1 << endl;

  if (flush) Flush();

}


std::pair<std::vector<BDWord>,
          std::vector<BDTime>> Driver::RecvTags(unsigned int core_id, unsigned int timeout_us) {

  typedef std::pair<std::vector<BDWord>, std::vector<BDTime>> RetType;

  RetType tat_tags;
  RetType acc_tags;

  // XXX need to have a timeout, otherwise we can hang even when we have something to send
  tat_tags = RecvFromEP(core_id, bdpars::BDFunnelEP::RO_TAT, timeout_us);
  acc_tags = RecvFromEP(core_id, bdpars::BDFunnelEP::RO_ACC, timeout_us);

  // concatenate
  tat_tags.first.insert(tat_tags.first.end()   , acc_tags.first.begin()  , acc_tags.first.end());
  tat_tags.second.insert(tat_tags.second.end() , acc_tags.second.begin() , acc_tags.second.end());

  return tat_tags;
}

void Driver::SendToEP(unsigned int core_id,
    uint8_t ep_code,
    const std::vector<BDWord>& payload,
    const std::vector<BDTime>& times) {

  auto serialized = std::make_unique<std::vector<EncInput>>();

  bool timed = times.size() > 0;

  // slightly different serialization behaviors for BD vs FPGA EPs
  // data width is 24 for BD EPs, 16 for FPGA EPs
  const unsigned int FPGA_serialization_width = bd_pars_->DnEPCodeIsBDHornEP(ep_code) ?
      FieldWidth(FPGAIO::PAYLOAD)
    : FieldWidth(THREEFPGAREGS::W0);

  const unsigned int ep_data_size = bd_pars_->Dn_EP_size_.at(ep_code);
  const unsigned int D = ep_data_size % FPGA_serialization_width == 0 ?
      ep_data_size / FPGA_serialization_width
    : ep_data_size / FPGA_serialization_width + 1;

  const unsigned int MaxD = 4;
  assert(D <= MaxD);

  unsigned int i = 0;
  for (auto& it : payload) {

    // convert from ns -> time units
    BDTime time = !timed ? 0 : NsToUnits(times.at(i)); // time 0 is never held up by the FPGA

    BDWord payloads[MaxD];

    // FPGA serialization scheme is lsbs first
    if (bd_pars_->DnEPCodeIsBDHornEP(ep_code)) {
      if (D == 1) {
        payloads[0] = it;
      } else if (D == 2) {
        payloads[0] = GetField(it, TWOFPGAPAYLOADS::LSB);
        payloads[1] = GetField(it, TWOFPGAPAYLOADS::MSB);
      } else {
        assert(false && "not implemented: no BD EP word has >2x FPGA serialization");
      }
    } else { // FPGA channel or register (all regs should be 1x, only channel is 4x)
      if (D == 1) {
        payloads[0] = it;
      } else if (D == 4) {
        payloads[0] = GetField(it, FOURFPGAREGS::W0);
        payloads[1] = GetField(it, FOURFPGAREGS::W1);
        payloads[2] = GetField(it, FOURFPGAREGS::W2);
        payloads[3] = GetField(it, FOURFPGAREGS::W3);
      } else {
        assert(false && "not implemented: no FPGA EP word has FPGA serialization != 1x or 4x");
      }
    }

    for (unsigned int j = 0; j < D; j++) {
      EncInput to_push;
      to_push.payload = payloads[j];

      to_push.time = time + j; // XXX note +j! THIS IS A HACK to avoid having serialized words get out of order!
                               // XXX the right way to fix this is to serialize AFTER sorting
      to_push.core_id = core_id;
      to_push.FPGA_ep_code = ep_code;
      serialized->push_back(to_push);
    }
    i++;
  }

  if (timed) {
    // push all the elements to the back of the vector then sort it
    for (auto& it : *serialized) {
      timed_queue_.push_back(it);
    }

    std::sort(timed_queue_.begin(), timed_queue_.end()); // (operator< is defined for EncInput)

    // update highest_ns_sent_
    BDTime new_highest_ns = timed_queue_.back().time * ns_per_unit_;
    highest_ns_sent_ = new_highest_ns > highest_ns_sent_ ? new_highest_ns : highest_ns_sent_; // max()

  } else {
    sequenced_queue_.push(std::move(serialized));
  }
}



std::pair<std::vector<BDWord>,
          std::vector<BDTime>>
  Driver::RecvFromEP(unsigned int core_id, uint8_t ep_code, unsigned int timeout_us) {

  // get data from buffer
  MutexBuffer<DecOutput>* this_buf = dec_bufs_out_.at(ep_code);
  std::vector<std::unique_ptr<std::vector<DecOutput>>> popped_data = this_buf->PopAll(timeout_us);

  std::vector<BDWord> words;
  std::vector<BDTime> times;

  // if there's a deserializer, use it
  for(auto& rit: popped_data){
    if (up_ep_deserializers_.count(ep_code) > 0) {
      auto& deserializer = up_ep_deserializers_.at(ep_code);
      deserializer->NewInput(std::move(rit));

      // read first word
      std::vector<DecOutput> deserialized; // continuosly write into here
      deserializer->GetOneOutput(&deserialized);
      while (deserialized.size() > 0) {
        // for now, D == 2 for all deserializers, so we can do this hack
        // if the width of a single data object returned from the FPGA ever
        // exceeds 64 bits, we may need to rethink this
        assert(deserialized.size() == 2); // the only case to deal with for now

        uint32_t payload_lsb = deserialized.at(0).payload;
        uint32_t payload_msb = deserialized.at(1).payload;
        BDTime   time        = deserialized.at(1).time; // take time on second word

        // concatenate lsb and msb to make output word
        BDWord payload_all = PackWord<TWOFPGAPAYLOADS>({{TWOFPGAPAYLOADS::LSB, payload_lsb}, {TWOFPGAPAYLOADS::MSB, payload_msb}});
        words.push_back(payload_all);
        times.push_back(UnitsToNs(time));

        deserializer->GetOneOutput(&deserialized);
      }
    } else {

      // otherwise, not much to do
      for (auto& it : *(rit.get())) {
        uint32_t payload = it.payload;
        BDTime   time    = it.time; // take time on second word

        words.push_back(payload);
        times.push_back(UnitsToNs(time));
      }
    }
  }

  return {words, times};
}

void Driver::SetBDRegister(unsigned int core_id, bdpars::BDHornEP reg_id, BDWord word, bool flush) {
  // form vector of values to set BDState's reg state with, in WordStructure field order
  assert(bd_pars_->BDHornEPIsReg(reg_id));
  bd_state_[core_id].SetReg(reg_id, word);
  SendToEP(core_id, reg_id, {word});
  if (flush) Flush();
}

void Driver::SetToggle(unsigned int core_id, bdpars::BDHornEP toggle_id, bool traffic_en, bool dump_en, bool flush) {
  //cout << "setting toggle at BDHornEP " << int(toggle_id) << " to traffic_en: " << int(traffic_en) << ", dump_en: " << int(dump_en) << endl;
  SetBDRegister(core_id, toggle_id, PackWord<ToggleWord>(
        {{ToggleWord::TRAFFIC_ENABLE, traffic_en}, {ToggleWord::DUMP_ENABLE, dump_en}}), flush);
}

} // bddriver
} // pystorm
