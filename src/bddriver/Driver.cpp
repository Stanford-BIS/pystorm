#include "Driver.h"

#include <cassert>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "comm/Comm.h"
#include "comm/CommSoft.h"
#include "common/BDPars.h"
#include "common/BDState.h"
#include "common/DriverPars.h"
#include "common/DriverTypes.h"
#include "common/MutexBuffer.h"
#include "common/binary_util.h"
#include "decoder/Decoder.h"
#include "encoder/Encoder.h"

using std::cout;
using std::endl;

namespace pystorm {
namespace bddriver {

constexpr uint64_t     FIFOInputTag::field_hard_values[];
constexpr unsigned int FIFOInputTag::field_widths[];
constexpr uint64_t     MMSetAddress::field_hard_values[];
constexpr unsigned int MMSetAddress::field_widths[];
constexpr uint64_t     MMReadIncrement::field_hard_values[];
constexpr unsigned int MMReadIncrement::field_widths[];
constexpr uint64_t     MMWriteIncrement::field_hard_values[];
constexpr unsigned int MMWriteIncrement::field_widths[];
constexpr uint64_t     AMSetAddress::field_hard_values[];
constexpr unsigned int AMSetAddress::field_widths[];
constexpr uint64_t     AMReadWrite::field_hard_values[];
constexpr unsigned int AMReadWrite::field_widths[];
constexpr uint64_t     AMIncrement::field_hard_values[];
constexpr unsigned int AMIncrement::field_widths[];
constexpr uint64_t     TATSetAddress::field_hard_values[];
constexpr unsigned int TATSetAddress::field_widths[];
constexpr uint64_t     TATReadIncrement::field_hard_values[];
constexpr unsigned int TATReadIncrement::field_widths[];
constexpr uint64_t     TATWriteIncrement::field_hard_values[];
constexpr unsigned int TATWriteIncrement::field_widths[];
constexpr uint64_t     PATRead::field_hard_values[];
constexpr unsigned int PATRead::field_widths[];
constexpr uint64_t     PATWrite::field_hard_values[];
constexpr unsigned int PATWrite::field_widths[];
constexpr uint64_t     AMEncapsulation::field_hard_values[];
constexpr unsigned int AMEncapsulation::field_widths[];
constexpr uint64_t     MMEncapsulation::field_hard_values[];
constexpr unsigned int MMEncapsulation::field_widths[];
constexpr uint64_t     DelayWord::field_hard_values[];
constexpr unsigned int DelayWord::field_widths[];
constexpr uint64_t     ToggleWord::field_hard_values[];
constexpr unsigned int ToggleWord::field_widths[];

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
  driver_pars_ = new driverpars::DriverPars();
  bd_pars_     = new bdpars::BDPars();

  // one BDState object per core
  // bd_state_ = std::vector<BDState>(bd_pars_->NumCores(), BDState(bd_pars_, driver_pars_));
  for (unsigned int i = 0; i < bd_pars_->NumCores(); i++) {
    bd_state_.push_back(BDState(bd_pars_, driver_pars_));
  }

  // initialize buffers
  enc_buf_in_  = new MutexBuffer<EncInput>(driver_pars_->Get(driverpars::ENC_BUF_IN_CAPACITY));
  enc_buf_out_ = new MutexBuffer<EncOutput>(driver_pars_->Get(driverpars::ENC_BUF_OUT_CAPACITY));
  dec_buf_in_  = new MutexBuffer<DecInput>(driver_pars_->Get(driverpars::DEC_BUF_IN_CAPACITY));

  for (unsigned int i = 0; i < bd_pars_->FunnelRoutes()->size(); i++) {
    MutexBuffer<DecOutput>* buf_ptr = new MutexBuffer<DecOutput>(driver_pars_->Get(driverpars::DEC_BUF_OUT_CAPACITY));
    dec_bufs_out_.push_back(buf_ptr);
  }

  // initialize Encoder and Decoder

  enc_ = new Encoder(
      bd_pars_,
      enc_buf_in_,
      enc_buf_out_,
      driver_pars_->Get(driverpars::ENC_CHUNK_SIZE),
      driver_pars_->Get(driverpars::ENC_TIMEOUT_US));

  dec_ = new Decoder(
      bd_pars_,
      dec_buf_in_,
      dec_bufs_out_,
      driver_pars_->Get(driverpars::DEC_CHUNK_SIZE),
      driver_pars_->Get(driverpars::DEC_TIMEOUT_US));

  // initialize Comm
  if (driver_pars_->Get(driverpars::COMM_TYPE) == driverpars::SOFT) {
    comm_ = new comm::CommSoft(
        *(driver_pars_->Get(driverpars::SOFT_COMM_IN_FNAME)),
        *(driver_pars_->Get(driverpars::SOFT_COMM_OUT_FNAME)),
        dec_buf_in_,
        enc_buf_out_);

  } else if (driver_pars_->Get(driverpars::COMM_TYPE) == driverpars::LIBUSB) {
    assert(false && "libUSB Comm is not implemented");
  } else if (driver_pars_->Get(driverpars::COMM_TYPE) == driverpars::BDMODEL) {
    // XXX hmm... this is iffy
    // should be using BDModelDriver, which handles this in its own ctor
    comm_ = nullptr;
  } else {
    assert(false && "unhandled comm_type");
  }
}

Driver::~Driver() {
  delete driver_pars_;
  delete bd_pars_;
  delete enc_buf_in_;
  delete enc_buf_out_;
  delete dec_buf_in_;
  for (MutexBuffer<DecOutput>* it : dec_bufs_out_) {
    delete it;
  }
  delete enc_;
  delete dec_;
  delete comm_;
}

void Driver::InitBD() {
  for (unsigned int i = 0; i < bd_pars_->NumCores(); i++) {
    // turn off traffic
    SetTagTrafficState(i, false);
    SetSpikeTrafficState(i, false);

    // Set the memory delays
    for(auto& mem : {bdpars::AM, bdpars::MM, bdpars::FIFO_PG, bdpars::FIFO_DCT, bdpars::TAT0, bdpars::TAT1, bdpars::PAT}) {
      const unsigned delay_val = 0;
      SetMemoryDelay(i, mem, delay_val, delay_val);
    }

    // init the FIFO
    InitFIFO(i);

    // clear all memories (perhaps unecessary? takes extra time)
    SetMem(i, bdpars::PAT,  std::vector<BDWord>(bd_pars_->Size(bdpars::PAT), BDWord(0)), 0);
    SetMem(i, bdpars::TAT0, std::vector<BDWord>(bd_pars_->Size(bdpars::TAT0), BDWord(0)), 0);
    SetMem(i, bdpars::TAT1, std::vector<BDWord>(bd_pars_->Size(bdpars::TAT1), BDWord(0)), 0);
    SetMem(i, bdpars::AM,   std::vector<BDWord>(bd_pars_->Size(bdpars::AM), BDWord(0)), 0);
    SetMem(i, bdpars::MM,   std::vector<BDWord>(bd_pars_->Size(bdpars::MM), BDWord(0)), 0);

    // XXX other stuff to do?
  }
}

void Driver::InitFIFO(unsigned int core_id) {
  // turn traffic off around FIFO (just kill everything to hijack the traffic drain timer in BDState)
  PauseTraffic(core_id);

  // make FIFO_HT head = tail (doesn't matter what you send)
  SendToHorn(core_id, bdpars::INIT_FIFO_HT, {0});

  // send all tag values to DCT FIF0 to dirty them so they flush
  std::vector<BDWord> all_tag_vals;
  for (unsigned int i = 0; i < bd_pars_->Size(bdpars::FIFO_DCT); i++) {
    all_tag_vals.push_back(BDWord::Create<FIFOInputTag>({{FIFOInputTag::TAG, i}}));
  }
  SendToHorn(core_id, bdpars::INIT_FIFO_DCT, all_tag_vals);

  // resume traffic will wait for the traffic drain timer before turning traffic regs back on
  ResumeTraffic(core_id);
}

void Driver::testcall(const std::string& msg) { std::cout << msg << std::endl; }

void Driver::Start() {
  // start all worker threads
  enc_->Start();
  dec_->Start();
  comm_->StartStreaming();
}

void Driver::Stop() {
  enc_->Stop();
  dec_->Stop();
  comm_->StopStreaming();
}

/// Set toggle traffic_en only, keep dump_en the same, returns previous traffic_en.
/// If register state has not been set, dump_en -> 0
bool Driver::SetToggleTraffic(unsigned int core_id, bdpars::RegId reg_id, bool en) {
  bool traffic_en, dump_en, reg_valid;
  std::tie(traffic_en, dump_en, reg_valid) = bd_state_[core_id].GetToggle(reg_id);
  if ((en != traffic_en) || !reg_valid) {
    SetToggle(core_id, reg_id, en, dump_en & reg_valid);
  }
  return traffic_en;
}

/// Set toggle dump_en only, keep traffic_en the same, returns previous dump_en.
/// If register state has not been set, traffic_en -> 0
bool Driver::SetToggleDump(unsigned int core_id, bdpars::RegId reg_id, bool en) {
  bool traffic_en, dump_en, reg_valid;
  std::tie(traffic_en, dump_en, reg_valid) = bd_state_[core_id].GetToggle(reg_id);
  if ((en != dump_en) || !reg_valid) {
    SetToggle(core_id, reg_id, traffic_en & reg_valid, en);
  }
  return dump_en;
}

/// Turn on tag traffic in datapath (also calls Start/KillSpikes)
void Driver::SetTagTrafficState(unsigned int core_id, bool en) {
  for (auto& it : kTrafficRegs) {
    SetToggleTraffic(core_id, it, en);
  }
}

/// Turn on spike outputs for all neurons
void Driver::SetSpikeTrafficState(unsigned int core_id, bool en) {
  SetToggleTraffic(core_id, bdpars::NEURON_DUMP_TOGGLE, en);
}

/// Turn on spike outputs for all neurons
void Driver::SetSpikeDumpState(unsigned int core_id, bool en) {
  SetToggleDump(core_id, bdpars::NEURON_DUMP_TOGGLE, en);
}

void Driver::PauseTraffic(unsigned int core_id) {
  assert(last_traffic_state_[core_id].size() == 0 && "called PauseTraffic twice before calling ResumeTraffic");
  last_traffic_state_[core_id] = {};
  for (auto& reg_id : kTrafficRegs) {
    bool last_state = SetToggleTraffic(core_id, reg_id, false);
    last_traffic_state_[core_id].push_back(last_state);
  }
  bd_state_[core_id].WaitForTrafficOff();
}

void Driver::ResumeTraffic(unsigned int core_id) {
  assert(last_traffic_state_[core_id].size() > 0 && "called ResumeTraffic before calling PauseTraffic");
  unsigned int i = 0;
  for (auto& reg_id : kTrafficRegs) {
    SetToggleTraffic(core_id, reg_id, last_traffic_state_[core_id][i]);
  }
  last_traffic_state_[core_id] = {};
}


void Driver::SetMemoryDelay(unsigned int core_id, bdpars::MemId mem_id, unsigned int read_value, unsigned int write_value) {
  BDWord word = BDWord::Create<DelayWord>({{DelayWord::READ_DELAY, read_value},
                                           {DelayWord::WRITE_DELAY, write_value}});
  SetRegister(core_id, bd_pars_->DelayRegForMem(mem_id), word);
}

void Driver::SetPreFIFODumpState(unsigned int core_id, bool dump_en) {
  SetToggleDump(core_id, bdpars::TOGGLE_PRE_FIFO, dump_en);
}

void Driver::SetPostFIFODumpState(unsigned int core_id, bool dump_en) {
  SetToggleDump(core_id, bdpars::TOGGLE_POST_FIFO0, dump_en);
  SetToggleDump(core_id, bdpars::TOGGLE_POST_FIFO1, dump_en);
}

std::vector<BDWord> Driver::GetFIFODump(unsigned int core_id, bdpars::OutputId output_id, unsigned int n_tags) {
  bdpars::FunnelLeafId leaf_id = bd_pars_->FunnelLeafIdFor(output_id);
  std::vector<BDWord> data = RecvFromFunnel(core_id, leaf_id, n_tags);
  return data;
}

std::vector<BDWord> Driver::GetPreFIFODump(unsigned int core_id, unsigned int n_tags) {
  return GetFIFODump(core_id, bdpars::PRE_FIFO_TAGS, n_tags);
}

std::pair<std::vector<BDWord>, std::vector<BDWord> > Driver::GetPostFIFODump(unsigned int core_id, unsigned int n_tags0, unsigned int n_tags1) {
  std::vector<BDWord> tags0, tags1;
  if (n_tags0 > 0) { // zero has special meaning: "grab something", which isn't what we want here
    tags0 = GetFIFODump(core_id, bdpars::POST_FIFO_TAGS0, n_tags0);
  }
  if (n_tags1 > 0) { // zero has special meaning: "grab something", which isn't what we want here
    tags1 = GetFIFODump(core_id, bdpars::POST_FIFO_TAGS1, n_tags1);
  }

  return std::make_pair(tags0, tags1);
}

std::pair<unsigned int, unsigned int> Driver::GetFIFOOverflowCounts(unsigned int core_id) {
  std::vector<BDWord> ovflw0 = RecvFromFunnel(core_id, bdpars::OVFLW0);
  std::vector<BDWord> ovflw1 = RecvFromFunnel(core_id, bdpars::OVFLW1);
  return {ovflw0.size(), ovflw1.size()};
}

void Driver::SetDACValue(unsigned int core_id, bdpars::DACSignalId signal_id, unsigned int value) {
  assert(false && "not implemented");  // fix the ADC connection state thing below
  bdpars::RegId DAC_reg_id = bd_pars_->DACSignalIdToDACRegisterId(signal_id);

  // look up state of connection to ADC XXX
  bool DAC_to_ADC_conn_curr_state = false;

  BDWord word = BDWord::Create<DACWord>({{DACWord::DAC_VALUE, value}, {DACWord::DAC_TO_ADC_CONN, DAC_to_ADC_conn_curr_state}});
  SetRegister(core_id, DAC_reg_id, word);
}

void Driver::SetDACtoADCConnectionState(unsigned int core_id, bdpars::DACSignalId dac_signal_id, bool en) {
  assert(false && "not implemented");
}

void DisconnectDACsfromADC(unsigned int core_id) {
  assert(false && "not implemented");
}

/// Set large/small current scale for either ADC
void Driver::SetADCScale(unsigned int core_id, bool adc_id, const std::string& small_or_large) {
  assert(false && "not implemented");
}

/// Turn ADC output on
void Driver::SetADCTrafficState(unsigned int core_id, bool en) {
  assert(false && "not implemented");
}

////////////////////////////////////////////////////////////////////////////////
// Neuron controls
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Soma controls
////////////////////////////////////////////////////////////////////////////////
void Driver::SetSomaEnableStatus(unsigned int soma_id,
    bdpars::SomaStatusId status) {
    assert(false && "not implemented");
}

void Driver::SetSomaGain(unsigned int soma_id, bdpars::SomaGainId soma_gain) {
    assert(false && "not implemented");
}

void Driver::SetSomaOffsetSign(unsigned int soma_id,
    bdpars::SomaOffsetSignId soma_offset_sign) {
    assert(false && "not implemented");
}

void Driver::SetSomaOffsetMultiplier(unsigned int soma_id,
    bdpars::SomaOffsetMultiplierId soma_offset_multiplier) {
    assert(false && "not implemented");
}

////////////////////////////////////////////////////////////////////////////////
// Synapse controls
////////////////////////////////////////////////////////////////////////////////
void Driver::SetSynapseEnableStatus(unsigned int soma_id,
    bdpars::SynapseStatusId status) {
    assert(false && "not implemented");
}

void Driver::SetSynapseADCStatus(unsigned int soma_id,
    bdpars::SynapseStatusId status) {
    assert(false && "not implemented");
}

////////////////////////////////////////////////////////////////////////////////
// Diffusor controls
////////////////////////////////////////////////////////////////////////////////
void Driver::SetDiffusorCutStatus(unsigned int tile_id,
    bdpars::DiffusorCutLocationId cut_id,
    bdpars::DiffusorCutStatusId status) {
    assert(false && "not implemented");
}

void Driver::SetDiffusorAllCutStatus(unsigned int tile_id,
    bdpars::DiffusorCutStatusId status) {
    assert(false && "not implemented");
}

void Driver::SetMem(
    unsigned int core_id,
    bdpars::MemId mem_id,
    const std::vector<BDWord> &data,
    unsigned int start_addr) {

  // update BDState
  bd_state_.at(core_id).SetMem(mem_id, start_addr, data);

  // depending on which memory this is, encapsulate differently
  std::vector<BDWord> encapsulated_words;
  if (mem_id == bdpars::PAT) {
    encapsulated_words = PackRWProgWords<PATWrite>(data, start_addr);
  } else if (mem_id == bdpars::TAT0 || mem_id == bdpars::TAT1) {
    encapsulated_words = PackRIWIProgWords<TATSetAddress, TATWriteIncrement>(data, start_addr);
  } else if (mem_id == bdpars::MM) {
    encapsulated_words = PackRIWIProgWords<MMSetAddress, MMWriteIncrement>(data, start_addr);
  } else if (mem_id == bdpars::AM) {
    encapsulated_words = PackRMWProgWords<AMSetAddress, AMReadWrite, AMIncrement>(data, start_addr);
  } else {
    assert(false && "Bad memory ID");
  }

  // if it's an AM or MM word, need further encapsulation
  if (mem_id == bdpars::MM) {
    encapsulated_words = PackAMMMWord<MMEncapsulation>(encapsulated_words);
  } else if (mem_id == bdpars::AM) {
    encapsulated_words = PackAMMMWord<AMEncapsulation>(encapsulated_words);
  }

  // transmit to horn
  PauseTraffic(core_id);
  SendToHorn(core_id, bd_pars_->HornLeafIdFor(mem_id), encapsulated_words);
  if (mem_id == bdpars::AM) { // if we're programming the AM, we're also dumping the AM, need to sink what comes back
    RecvFromFunnel(core_id, bd_pars_->FunnelLeafIdFor(mem_id), data.size());
  }
  ResumeTraffic(core_id);
}


std::vector<BDWord> Driver::DumpMem(unsigned int core_id, bdpars::MemId mem_id) {

  // make dump words
  unsigned int mem_size = bd_pars_->Size(mem_id);

  std::vector<BDWord> encapsulated_words;
  if (mem_id == bdpars::PAT) {
    encapsulated_words = PackRWDumpWords<PATRead>(0, mem_size);
  } else if (mem_id == bdpars::TAT0 || mem_id == bdpars::TAT1) {
    encapsulated_words = PackRIWIDumpWords<TATSetAddress, TATReadIncrement>(0, mem_size);
  } else if (mem_id == bdpars::MM) {
    encapsulated_words = PackRIWIDumpWords<MMSetAddress, MMReadIncrement>(0, mem_size);
  } else if (mem_id == bdpars::AM) {
    // a little tricky, reprogramming is the same as dump
    // need to write back whatever is currently in memory
    const std::vector<BDWord> *curr_data = bd_state_.at(core_id).GetMem(bdpars::AM);
    encapsulated_words = PackRMWProgWords<AMSetAddress, AMReadWrite, AMIncrement>(*curr_data, 0);
  } else {
    assert(false && "Bad memory ID");
  }

  // if it's an AM or MM word, need further encapsulation
  if (mem_id == bdpars::MM) {
    encapsulated_words = PackAMMMWord<MMEncapsulation>(encapsulated_words);
  } else if (mem_id == bdpars::AM) {
    encapsulated_words = PackAMMMWord<AMEncapsulation>(encapsulated_words);
  }

  // transmit read words, then block until all dump words have been received
  // XXX if something goes terribly wrong and not all the words come back, this will hang
  bdpars::HornLeafId horn_leaf = bd_pars_->HornLeafIdFor(mem_id);
  bdpars::FunnelLeafId funnel_leaf = bd_pars_->FunnelLeafIdFor(mem_id);
  PauseTraffic(core_id);
  SendToHorn(core_id, horn_leaf, encapsulated_words);
  std::vector<BDWord> payloads = RecvFromFunnel(core_id, funnel_leaf, mem_size);
  ResumeTraffic(core_id);

  // unpack payload field of DecOutput according to word format
  return payloads;
}

template <class TWrite>
std::vector<BDWord> Driver::PackRWProgWords(const std::vector<BDWord>& payload, unsigned int start_addr) const
{
  std::vector<BDWord> retval;

  uint64_t addr = static_cast<uint64_t>(start_addr);
  for (auto& it : payload) {
    retval.push_back(BDWord::Create<TWrite>({{TWrite::ADDRESS, addr}, {TWrite::DATA, it.Packed()}}));
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
    retval.push_back(BDWord::Create<TRead>({{TRead::ADDRESS, addr}}));
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
  retval.push_back(BDWord::Create<TSet>({{TSet::ADDRESS, start_addr}}));

  for (auto& it : payload) {
    retval.push_back(BDWord::Create<TWrite>({{TWrite::DATA, it.Packed()}}));
  }

  return retval;
}

template <class TSet, class TRead>
std::vector<BDWord> Driver::PackRIWIDumpWords(
    unsigned int start_addr, unsigned int end_addr) const
{
  std::vector<BDWord> retval;

  // address word
  retval.push_back(BDWord::Create<TSet>({{TSet::ADDRESS, start_addr}}));

  for (unsigned int i = 0; i < end_addr - start_addr; i++) {
    retval.push_back(BDWord::Create<TRead>({{}}));
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
  retval.push_back(BDWord::Create<TSet>({{TSet::ADDRESS, addr}}));

  for (auto& it : payload) {
    retval.push_back(BDWord::Create<TReadWrite>({{TReadWrite::DATA, it.Packed()}}));
    retval.push_back(BDWord::Create<TInc>({{}}));
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

    BDWord word = BDWord::Create<AMorMMEncapsulation>(
          {{AMorMMEncapsulation::PAYLOAD, payload_data[i].Packed()}, {AMorMMEncapsulation::AMMM_STOP, stop_bit}});
    retval.push_back(word);
  }
  return retval;
}

void Driver::SendSpikes(const std::vector<unsigned int>& core_ids, const std::vector<BDWord>& spikes, const std::vector<BDTime> times) {
  assert(core_ids.size() == spikes.size());
  assert(core_ids.size() == times.size());

  // XXX this circumvents SendToHorn, don't want to hit the serialization code

  // get reference to enc_buf_in_'s memory
  EncInput* write_to = enc_buf_in_->LockBack(spikes.size());


  unsigned int leaf_id_as_uint = bd_pars_->HornIdx(bd_pars_->HornLeafIdFor(bdpars::INPUT_SPIKES));

  for (unsigned int i = 0; i < spikes.size(); i++) {
    // XXX throwing times on the ground for now
    write_to[i] = {core_ids[i], leaf_id_as_uint, spikes[i].Packed<uint32_t>()};
  }

  // unlock
  enc_buf_in_->UnlockBack();
}

void Driver::SendTags(const std::vector<unsigned int>& core_ids, const std::vector<BDWord>& tags, const std::vector<BDTime> times) {
  assert(core_ids.size() == tags.size());
  assert(core_ids.size() == times.size());

  // XXX this circumvents SendToHorn, don't want to hit the serialization code

  // get reference to enc_buf_in_'s memory
  EncInput* write_to = enc_buf_in_->LockBack(tags.size());

  unsigned int leaf_id_as_uint = bd_pars_->HornIdx(bd_pars_->HornLeafIdFor(bdpars::INPUT_TAGS));

  for (unsigned int i = 0; i <tags.size(); i++) {
    // XXX throwing times on the ground for now
    write_to[i] = {core_ids[i], leaf_id_as_uint, tags[i].Packed<uint32_t>()};
  }
  // unlock
  enc_buf_in_->UnlockBack();
}


std::tuple<std::vector<unsigned int>,
          std::vector<BDWord>,
          std::vector<BDTime> > Driver::RecvSpikes(unsigned int max_to_recv) {

  // XXX this circumvents RecvFromFunnel, don't want to hit the serialization code

  unsigned int buf_idx = bd_pars_->FunnelIdx(bd_pars_->FunnelLeafIdFor(bdpars::OUTPUT_SPIKES));
  unsigned int timeout = driver_pars_->Get(driverpars::RECVSPIKES_TIMEOUT_US);

  const DecOutput *MB_front;
  unsigned int num_readable;
  std::tie(MB_front, num_readable) = dec_bufs_out_[buf_idx]->LockFront(max_to_recv, timeout);

  std::vector<unsigned int> core_ids;
  std::vector<BDWord> spikes;
  std::vector<unsigned int> times;
  for (unsigned int i = 0; i < num_readable; i++) {
    DecOutput this_spike = MB_front[i];
    spikes.push_back(BDWord(this_spike.payload));
    core_ids.push_back(this_spike.core_id);
    times.push_back(0);
  }

  return make_tuple(core_ids, spikes, times);
}

std::tuple<std::vector<unsigned int>,
          std::vector<BDWord>,
          std::vector<BDTime> > Driver::RecvTags(unsigned int max_to_recv) {

  // XXX this circumvents RecvFromFunnel, don't want to hit the serialization code
  //
  std::vector<unsigned int> core_ids;
  std::vector<BDWord> tags;
  std::vector<unsigned int> times;
  unsigned int num_to_recv_remaining = max_to_recv;
  for (auto& output_id : {bdpars::TAT_OUTPUT_TAGS, bdpars::ACC_OUTPUT_TAGS}) {
    unsigned int buf_idx = bd_pars_->FunnelIdx(bd_pars_->FunnelLeafIdFor(output_id));
    unsigned int timeout = driver_pars_->Get(driverpars::RECVTAGS_TIMEOUT_US);

    const DecOutput *MB_front;
    unsigned int num_readable;
    std::tie(MB_front, num_readable) = dec_bufs_out_[buf_idx]->LockFront(num_to_recv_remaining, timeout);
    for (unsigned int i = 0; i < num_readable; i++) {
      DecOutput this_tag = MB_front[i];

      tags.push_back(BDWord(this_tag.payload));
      core_ids.push_back(this_tag.core_id);
      times.push_back(0);
    }
    dec_bufs_out_[buf_idx]->UnlockFront();
  }

  return make_tuple(core_ids, tags, times);
}

std::pair<std::vector<uint32_t>, unsigned int> Driver::SerializeWordsToLeaf(
    const std::vector<uint64_t>& inputs, bdpars::HornLeafId leaf_id) const {
  unsigned int input_width   = bd_pars_->Width(leaf_id);
  unsigned int serialization = bd_pars_->Serialization(leaf_id);

  return SerializeWords<uint64_t, uint32_t>(inputs, input_width, serialization);
}

std::pair<std::vector<uint64_t>, std::vector<uint32_t> > Driver::DeserializeWordsFromLeaf(
    const std::vector<uint32_t>& inputs, bdpars::FunnelLeafId leaf_id) const {
  unsigned int deserialization    = bd_pars_->Serialization(leaf_id);
  unsigned int deserialized_width = bd_pars_->Width(leaf_id);

  return DeserializeWords<uint32_t, uint64_t>(inputs, deserialized_width, deserialization);
}

void Driver::SendToHorn(unsigned int core_id, bdpars::HornLeafId leaf_id, const std::vector<BDWord>& payload) {
  // convert to uint, should maybe just reinterpret cast
  std::vector<uint64_t> raw_payloads;
  for (auto& it : payload) {
    raw_payloads.push_back(it.Packed());
  }

  // do serialization
  std::vector<uint32_t> serialized_words;
  unsigned int serialized_width;
  std::tie(serialized_words, serialized_width) = SerializeWordsToLeaf(raw_payloads, leaf_id);

  // Encoder doesn't know about the enums, cast leaf_id as a uint
  unsigned int leaf_id_as_uint = static_cast<unsigned int>(leaf_id);

  // have to make sure that we don't send something bigger than the buffer
  std::vector<EncInput> enc_inputs;
  unsigned int i = 0;
  while (i < serialized_words.size()) {
    // package into EncInput
    enc_inputs.push_back({core_id, leaf_id_as_uint, serialized_words[i]});
    i++;
    if (i % driver_pars_->Get(driverpars::ENC_BUF_IN_CAPACITY) == 0) {
      enc_buf_in_->Push(enc_inputs);
      enc_inputs.clear();
    }
  }
  enc_buf_in_->Push(enc_inputs);  // push any remainder
}


std::vector<BDWord> Driver::RecvFromFunnel(unsigned int core_id, bdpars::FunnelLeafId leaf_id, unsigned num_to_recv) {
  // This is a call of convenience. Often, we're interested not just in a particular
  // leaf's traffic, but also just the traffic from a particular core.
  //
  // This isn't implemented for the multi-core case.
  // That's complicated, and probably needs some additional interfaces
  // in the MB to do some fancy locking,
  // or we need to dynamically allocate MBs by core.

  // Decoder doesn't know about enums, cast leaf_id as uint
  // this is the index of the dec_bufs_out[] we want to pull from
  unsigned int leaf_id_as_uint     = static_cast<unsigned int>(leaf_id);
  MutexBuffer<DecOutput>* this_buf = dec_bufs_out_.at(leaf_id_as_uint);

  // look up serialization, we really need num_to_recv * serialiazation
  unsigned int deserialization = bd_pars_->Serialization(leaf_id);

  // Pop <num_to_recv> * deserialization elements from <leaf_id>'s buffer to outputs
  std::vector<DecOutput> outputs;

  if (num_to_recv > 0) {
    unsigned int DecOutput_needed = num_to_recv * deserialization;

    while (outputs.size() < DecOutput_needed) {
      unsigned int num_to_pop = DecOutput_needed - outputs.size();
      std::vector<DecOutput> new_outputs = this_buf->PopVect(num_to_pop, 0, deserialization);
      outputs.insert(outputs.end(), new_outputs.begin(), new_outputs.end());
    }
  } else {
    // if num_to_recv=0, just take whatever's in the queue
    unsigned int num_to_pop = driver_pars_->Get(driverpars::DEC_BUF_OUT_CAPACITY);
    outputs = this_buf->PopVect(num_to_pop, 0, deserialization);
  }

  // deserialize (pull out payloads first)
  std::vector<uint32_t> payloads;
  for (auto& output : outputs) {
    payloads.push_back(output.payload);
    // throw the time on the ground, if you want it, you're not using this call
    assert(output.core_id == core_id && "not implmented for multi-core");
  }

  std::vector<uint64_t> deserialized_payloads;
  std::vector<uint32_t> remainder;
  std::tie(deserialized_payloads, remainder) = DeserializeWordsFromLeaf(payloads, leaf_id);
  assert(remainder.size() == 0);

  std::vector<BDWord> words; // would be better to reinterperet...?
  for (auto& payload : deserialized_payloads) {
    words.push_back(BDWord(payload));
  }

  return words;
}

void Driver::SetRegister(unsigned int core_id, bdpars::RegId reg_id, BDWord word) {
  // form vector of values to set BDState's reg state with, in WordStructure field order
  bd_state_[core_id].SetReg(reg_id, word);
  SendToHorn(core_id, bd_pars_->HornLeafIdFor(reg_id), {word});
}

void Driver::SetToggle(unsigned int core_id, bdpars::RegId toggle_id, bool traffic_en, bool dump_en) {
  SetRegister(core_id, toggle_id, BDWord::Create<ToggleWord>(
        {{ToggleWord::TRAFFIC_ENABLE, traffic_en}, {ToggleWord::DUMP_ENABLE, dump_en}}));
}

} // bddriver
} // pystorm
