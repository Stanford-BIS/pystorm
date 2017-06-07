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
    // clear all memories
    SetPAT(i, std::vector<PATData>(bd_pars_->Size(bdpars::PAT), {0, 0, 0}), 0);
    SetTAT(i, 0, std::vector<TATData>(bd_pars_->Size(bdpars::TAT0), {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}), 0);
    SetTAT(i, 1, std::vector<TATData>(bd_pars_->Size(bdpars::TAT1), {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}), 0);
    SetAM(i, std::vector<AMData>(bd_pars_->Size(bdpars::AM), {0, 0, 0}), 0);
    SetMM(i, std::vector<MMData>(bd_pars_->Size(bdpars::MM), 0), 0);

    SetTagTrafficState(i, false);

    // XXX theres a lot of other stuff that should happen, e.g. InitFIFO
  }
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

void Driver::SetDACValue(unsigned int core_id, bdpars::DACSignalId signal_id, unsigned int value) {
  assert(false && "not implemented");  // fix the ADC connection state thing below
  bdpars::RegId DAC_reg_id = bd_pars_->DACSignalIdToDACRegisterId(signal_id);

  // look up state of connection to ADC XXX
  bool DAC_to_ADC_conn_curr_state = false;

  FieldValues field_vals = {{bdpars::DAC_VALUE, value}, {bdpars::DAC_TO_ADC_CONN, DAC_to_ADC_conn_curr_state}};
  SetRegister(core_id, DAC_reg_id, field_vals);
}

void Driver::SetDACtoADCConnectionState(unsigned int core_id, bdpars::DACSignalId dac_signal_id, bool en) {
  assert(false && "not implemented");
}

void DisconnectDACsfromADC(unsigned int core_id) { assert(false && "not implemented"); }

/// Set large/small current scale for either ADC
void Driver::SetADCScale(unsigned int core_id, bool adc_id, const std::string& small_or_large) {
  assert(false && "not implemented");
}

/// Turn ADC output on
void Driver::SetADCTrafficState(unsigned int core_id, bool en) { assert(false && "not implemented"); }

void Driver::SetMem(
    unsigned int core_id,
    bdpars::MemId mem_id,
    const VFieldValues &data,
    unsigned int start_addr) {

  // pack data fields, TAT is special
  std::vector<uint64_t> data_fields;
  if (mem_id == bdpars::TAT0 || mem_id == bdpars::TAT1) { 
    unsigned int TAT_word_type;
    for (auto& fv : data) {
      if (FVContains(fv, bdpars::AM_ADDRESS))             TAT_word_type = 0;
      else if (FVContains(fv, bdpars::SYNAPSE_ADDRESS_0)) TAT_word_type = 1;
      else if (FVContains(fv, bdpars::TAG))               TAT_word_type = 2;
      data_fields.push_back(PackWord(*bd_pars_->Word(mem_id, TAT_word_type), fv));
    }
  } else {
    data_fields = PackWords(*bd_pars_->Word(mem_id), data);
  }

  // depending on which memory this is, encapsulate differently
  std::vector<uint64_t> encapsulated_words;
  if (mem_id == bdpars::PAT) {
    encapsulated_words = PackRWProgWords(*(bd_pars_->Word(bdpars::PAT_WRITE)), data_fields, start_addr);
  } else if (mem_id == bdpars::TAT0 || mem_id == bdpars::TAT1) {
    encapsulated_words = PackRIWIProgWords(
        *(bd_pars_->Word(bdpars::TAT_SET_ADDRESS)),
        *(bd_pars_->Word(bdpars::TAT_WRITE_INCREMENT)),
        data_fields,
        start_addr);
  } else if (mem_id == bdpars::MM) {
    encapsulated_words = PackRIWIProgWords(
        *(bd_pars_->Word(bdpars::MM_SET_ADDRESS)),
        *(bd_pars_->Word(bdpars::MM_WRITE_INCREMENT)),
        data_fields,
        start_addr);
  } else if (mem_id == bdpars::AM) {
    encapsulated_words = PackRMWProgWords(
        *(bd_pars_->Word(bdpars::AM_SET_ADDRESS)),
        *(bd_pars_->Word(bdpars::AM_READ_WRITE)),
        *(bd_pars_->Word(bdpars::AM_INCREMENT)),
        data_fields,
        start_addr);
  }

  // if it's an AM or MM word, need further encapsulation
  if (mem_id == bdpars::MM || mem_id == bdpars::AM) {
    encapsulated_words = PackAMMMWord(mem_id, encapsulated_words);
  }

  // transmit to horn
  PauseTraffic(core_id);
  SendToHorn(core_id, bd_pars_->HornLeafIdFor(mem_id), encapsulated_words);
  ResumeTraffic(core_id);
}


VFieldValues Driver::DumpMem(unsigned int core_id, bdpars::MemId mem_id) {

  // make dump words
  unsigned int mem_size = bd_pars_->Size(mem_id);

  std::vector<uint64_t> read_words;
  switch (mem_id) {
    case bdpars::PAT: {
      read_words = PackRWDumpWords(*(bd_pars_->Word(bdpars::PAT_READ)), 0, mem_size);
      break;
    }
    case bdpars::TAT0: {
      read_words = PackRIWIDumpWords(*(bd_pars_->Word(bdpars::TAT_SET_ADDRESS)), *(bd_pars_->Word(bdpars::TAT_READ_INCREMENT)), 0, mem_size);
      break;
    }
    case bdpars::TAT1: {
      read_words = PackRIWIDumpWords(*(bd_pars_->Word(bdpars::TAT_SET_ADDRESS)), *(bd_pars_->Word(bdpars::TAT_READ_INCREMENT)), 0, mem_size);
      break;
    }
    case bdpars::MM: {
      read_words = PackRIWIDumpWords(*(bd_pars_->Word(bdpars::MM_SET_ADDRESS)), *(bd_pars_->Word(bdpars::MM_READ_INCREMENT)), 0, mem_size);
      break;
    }
    case bdpars::AM: {
      // a little tricky, reprogramming is the same as dump
      // need to write back whatever is currently in memory
      std::vector<uint64_t> curr_data_fields = PackWords(
          *bd_pars_->Word(mem_id), 
          DataToVFieldValues(*bd_state_.at(core_id).GetAM()));
      read_words = PackRMWProgWords(
          *(bd_pars_->Word(bdpars::AM_SET_ADDRESS)), 
          *(bd_pars_->Word(bdpars::AM_READ_WRITE)), 
          *(bd_pars_->Word(bdpars::AM_INCREMENT)), 
          curr_data_fields,
          0);
      break;
    }
    default: {
      assert(false);
      break;
    }
  }

  // transmit read words, then block until all dump words have been received
  // XXX if something goes terribly wrong and not all the words come back, this will hang
  bdpars::HornLeafId horn_leaf = bd_pars_->HornLeafIdFor(mem_id);
  bdpars::FunnelLeafId funnel_leaf = bd_pars_->FunnelLeafIdFor(mem_id);
  PauseTraffic(core_id);
  SendToHorn(core_id, horn_leaf, read_words);
  std::vector<uint64_t> payloads = RecvFromFunnel(funnel_leaf, core_id, mem_size);
  ResumeTraffic(core_id);

  // unpack payload field of DecOutput according to pat word format
  VFieldValues fields = UnpackWords(*bd_pars_->Word(mem_id), payloads);
  return fields;
}

std::vector<uint64_t> Driver::PackRWProgWords(
    const bdpars::WordStructure& word_struct, const std::vector<uint64_t>& payload, unsigned int start_addr) const
// NOTE: word_struct is a parameter because memories with the same type may still have different field widths
// XXX should vectorize, use PackWords
{
  std::vector<uint64_t> retval;

  uint64_t addr = static_cast<uint64_t>(start_addr);
  for (auto& it : payload) {
    retval.push_back(PackWord(word_struct, {{bdpars::ADDRESS, addr}, {bdpars::DATA, it}}));
    addr++;
  }

  return retval;
}

std::vector<uint64_t> Driver::PackRWDumpWords(
    const bdpars::WordStructure& word_struct, unsigned int start_addr, unsigned int end_addr) const
// XXX should vectorize, use PackWords
{
  std::vector<uint64_t> retval;
  for (unsigned int addr = start_addr; addr < end_addr; addr++) {
    retval.push_back(PackWord(word_struct, {{bdpars::ADDRESS, static_cast<uint64_t>(addr)}}));
  }
  return retval;
}

std::vector<uint64_t> Driver::PackRIWIProgWords(
    const bdpars::WordStructure& addr_word_struct,
    const bdpars::WordStructure& write_word_struct,
    const std::vector<uint64_t>& payload,
    unsigned int start_addr) const
// NOTE: word_struct is a parameter because memories with the same type may still have different field widths
// XXX should vectorize, use PackWords
{
  std::vector<uint64_t> retval;

  // address word
  uint64_t addr = static_cast<uint64_t>(start_addr);
  retval.push_back(PackWord(addr_word_struct, {{bdpars::ADDRESS, addr}}));

  for (auto& it : payload) {
    retval.push_back(PackWord(write_word_struct, {{bdpars::DATA, it}}));
  }

  return retval;
}

std::vector<uint64_t> Driver::PackRIWIDumpWords(
    const bdpars::WordStructure& addr_word_struct,
    const bdpars::WordStructure& read_word_struct,
    unsigned int start_addr,
    unsigned int end_addr) const
// XXX should vectorize, use PackWords
{
  std::vector<uint64_t> retval;
  retval.push_back(PackWord(addr_word_struct, {{bdpars::ADDRESS, static_cast<uint64_t>(start_addr)}}));
  for (unsigned int addr = start_addr; addr < end_addr; addr++) {
    retval.push_back(PackWord(read_word_struct, {}));
  }
  return retval;
}

std::vector<uint64_t> Driver::PackRMWProgWords(
    const bdpars::WordStructure& addr_word_struct,
    const bdpars::WordStructure& write_word_struct,
    const bdpars::WordStructure& incr_word_struct,
    const std::vector<uint64_t>& payload,
    unsigned int start_addr) const
// XXX should vectorize, use PackWords
{
  // RMWProgWord == RMWDumpWord

  std::vector<uint64_t> retval;

  // address word
  uint64_t addr = static_cast<uint64_t>(start_addr);
  retval.push_back(PackWord(addr_word_struct, {{bdpars::ADDRESS, addr}}));

  for (auto& it : payload) {
    retval.push_back(PackWord(write_word_struct, {{bdpars::DATA, it}}));
    retval.push_back(PackWord(incr_word_struct, {}));  // only fixed value of increment op for this word
  }

  return retval;
}

std::vector<uint64_t> Driver::PackAMMMWord(bdpars::MemId AM_or_MM, const std::vector<uint64_t>& payload_data) const
// XXX should vectorize, use PackWords
{
  // the reason for a stop bit type is a bit of a hack
  // see BDModel, Process() needs to be able to differentiate between AM/MM
  // collapsed FVVs
  const bdpars::WordStructure* AM_MM_encapsulation;
  if (AM_or_MM == bdpars::AM) {
    AM_MM_encapsulation = bd_pars_->Word(bdpars::AM_ENCAPSULATION);
  } else if (AM_or_MM == bdpars::MM) {
    AM_MM_encapsulation = bd_pars_->Word(bdpars::MM_ENCAPSULATION);
  } else {
    assert(false && "AM_or_MM must == AM or MM");
    return {};
  }

  std::vector<uint64_t> retval;
  for (unsigned int i = 0; i < payload_data.size(); i++) {
    uint64_t stop_bit;
    if (i == payload_data.size() - 1) {
      stop_bit = 1;
    } else {
      stop_bit = 0;
    }

    uint64_t word = PackWord(*AM_MM_encapsulation, {{bdpars::PAYLOAD, payload_data[i]}, {bdpars::AMMM_STOP, stop_bit}});
    retval.push_back(word);
  }
  return retval;
}

void Driver::SendSpikes(const std::vector<SynSpike>& spikes) {
  // XXX this circumvents going through the normal set of calls:
  // no VFieldValues/PackWords and no SerializeWords

  // We look up the width of the synapse and sign fields,
  // but we are hardcoding the order
  const bdpars::WordStructure* spike_word_struct = bd_pars_->Word(bdpars::INPUT_SPIKES);
  unsigned int widths[2]                         = {spike_word_struct->at(0).second, spike_word_struct->at(1).second};

  // we can at least assert that what we're writing is consistent with BDPars
  // if BDPars were to change, we want to know about it
  assert(spike_word_struct->at(0).first == bdpars::SYNAPSE_SIGN);
  assert(spike_word_struct->at(1).first == bdpars::SYNAPSE_ADDRESS);

  // get reference to enc_buf_in_'s memory
  EncInput* write_to = enc_buf_in_->LockBack(spikes.size());
  for (unsigned int i = 0; i < spikes.size(); i++) {
    // XXX time? need to figure out what to do in the FPGA
    // probably need to insert delay events here

    uint32_t sign_and_synapse_id[2] = {static_cast<uint32_t>(SignedValToSignBit(spikes[i].sign)), spikes[i].synapse_id};
    uint32_t payload                = Pack32(sign_and_synapse_id, widths, 2);

    unsigned int leaf_id_as_uint = bd_pars_->HornIdx(bd_pars_->HornLeafIdFor(bdpars::INPUT_SPIKES));

    // write to memory pointed to by LockBack()
    write_to[i] = {spikes[i].core_id, leaf_id_as_uint, payload};
  }
  // unlock
  enc_buf_in_->UnlockBack();
}

void Driver::SendTags(const std::vector<Tag> &tags) {

  // look up word structure of tag to use later in packing
  const bdpars::WordStructure * tag_word_struct = bd_pars_->Word(bdpars::INPUT_TAGS);

  // get reference to enc_buf_in_'s memory
  EncInput* write_to = enc_buf_in_->LockBack(tags.size());
  for (unsigned int i = 0; i <tags.size(); i++) {
    // XXX time? need to figure out what to do in the FPGA
    // probably need to insert delay events here

    FieldValues fv = DataToFieldValues(tags[i]);
    uint32_t payload = static_cast<uint32_t>(PackWord(*tag_word_struct, fv));

    unsigned int leaf_id_as_uint = bd_pars_->HornIdx(bd_pars_->HornLeafIdFor(bdpars::INPUT_TAGS));

    // write to memory pointed to by LockBack()
    write_to[i] = {tags[i].core_id, leaf_id_as_uint, payload};
  }
  // unlock
  enc_buf_in_->UnlockBack();
}


std::vector<NrnSpike> Driver::RecvSpikes(unsigned int max_to_recv) {
  unsigned int buf_idx = bd_pars_->FunnelIdx(bd_pars_->FunnelLeafIdFor(bdpars::OUTPUT_SPIKES));
  unsigned int timeout = driver_pars_->Get(driverpars::RECVSPIKES_TIMEOUT_US);

  const DecOutput *MB_front;
  unsigned int num_readable;
  std::tie(MB_front, num_readable) = dec_bufs_out_[buf_idx]->LockFront(max_to_recv, timeout);

  std::vector<NrnSpike> retval;
  for (unsigned int i = 0; i < num_readable; i++) {
    DecOutput this_spike = MB_front[i];
    // spikes don't have any data fields, payload == nrn addr
    retval.push_back({this_spike.time_epoch, this_spike.core_id, this_spike.payload});
  }
  
  return retval;
}

std::vector<Tag> Driver::RecvTags(unsigned int max_to_recv) {

  // look up word structure of tag to use later in unpacking
  const bdpars::WordStructure * tag_word_struct = bd_pars_->Word(bdpars::TAT_OUTPUT_TAGS);
  
  // receive from both tag output leaves, the acc and TAT
  std::vector<Tag> retval;
  unsigned int num_to_recv_remaining = max_to_recv;
  for (auto& output_id : {bdpars::TAT_OUTPUT_TAGS, bdpars::ACC_OUTPUT_TAGS}) {
    unsigned int buf_idx = bd_pars_->FunnelIdx(bd_pars_->FunnelLeafIdFor(output_id));
    unsigned int timeout = driver_pars_->Get(driverpars::RECVTAGS_TIMEOUT_US);

    const DecOutput *MB_front;
    unsigned int num_readable;
    std::tie(MB_front, num_readable) = dec_bufs_out_[buf_idx]->LockFront(num_to_recv_remaining, timeout);
    for (unsigned int i = 0; i < num_readable; i++) {
      DecOutput this_tag = MB_front[i];

      // XXX could dodge Unpack'ing by hardcoding
      // unpack tag and count
      FieldValues fv = UnpackWord(*tag_word_struct, this_tag.payload);
      retval.push_back(FieldValuesToTag(fv, this_tag.time_epoch, this_tag.core_id));
           
    }
    dec_bufs_out_[buf_idx]->UnlockFront();
  }

  return retval;
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

void Driver::SendToHorn(unsigned int core_id, bdpars::HornLeafId leaf_id, const std::vector<uint64_t>& payload) {
  // do serialization
  std::vector<uint32_t> serialized_words;
  unsigned int serialized_width;
  std::tie(serialized_words, serialized_width) = SerializeWordsToLeaf(payload, leaf_id);

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

std::vector<uint64_t> Driver::RecvFromFunnel(bdpars::FunnelLeafId leaf_id, unsigned int core_id, unsigned num_to_recv) {
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
  MutexBuffer<DecOutput>* this_buf = dec_bufs_out_[leaf_id_as_uint];

  // look up serialization, we really need num_to_recv * serialiazation
  unsigned int deserialization = bd_pars_->Serialization(leaf_id);

  // Pop <num_to_recv> * deserialization elements from <leaf_id>'s buffer to outputs
  std::vector<DecOutput> outputs;

  if (num_to_recv > 0) {
    unsigned int DecOutput_needed = num_to_recv * deserialization;

    while (outputs.size() < DecOutput_needed) {
      unsigned int num_to_pop = DecOutput_needed - outputs.size();
      this_buf->Pop(&outputs, num_to_pop, 0, deserialization);
    }
  } else {
    // if num_to_recv=0, just take whatever's in the queue
    unsigned int num_to_pop = driver_pars_->Get(driverpars::DEC_BUF_OUT_CAPACITY);
    this_buf->Pop(&outputs, num_to_pop, 0, deserialization);
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

  return deserialized_payloads;
}

void Driver::SetRegister(unsigned int core_id, bdpars::RegId reg_id, const FieldValues& field_vals) {
  const bdpars::WordStructure* reg_word_struct = bd_pars_->Word(reg_id);
  uint64_t payload                             = PackWord(*reg_word_struct, field_vals);

  // form vector of values to set BDState's reg state with, in WordStructure field order
  std::vector<unsigned int> field_vals_as_vect;
  for (auto& it : *reg_word_struct) {
    bdpars::WordFieldId field_id = it.first;
    field_vals_as_vect.push_back(FVGet(field_vals, field_id));
  }
  bd_state_[core_id].SetReg(reg_id, field_vals_as_vect);

  SendToHorn(core_id, bd_pars_->HornLeafIdFor(reg_id), {payload});
}

void Driver::SetToggle(unsigned int core_id, bdpars::RegId toggle_id, bool traffic_en, bool dump_en) {
  SetRegister(core_id, toggle_id, {{bdpars::TRAFFIC_ENABLE, traffic_en}, {bdpars::DUMP_ENABLE, dump_en}});
}

}  // bddriver namespace
}  // pystorm namespace
