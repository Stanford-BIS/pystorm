#include "Driver.h"

#include <iostream>
#include <vector>
#include <unordered_map>
#include <cassert>

#include "common/DriverTypes.h"
#include "common/BDPars.h"
#include "common/BDState.h"
#include "common/DriverPars.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include "common/MutexBuffer.h"
#include "common/binary_util.h"
#include "comm/Comm.h"
#include "comm/CommSoft.h"

using std::cout;
using std::endl;

namespace pystorm {
namespace bddriver {


//Driver * Driver::GetInstance()
//{
//    // In C++11, if control from two threads occurs concurrently, execution
//    // shall wait during static variable initialization, therefore, this is 
//    // thread safe
//    static Driver m_instance;
//    return &m_instance;
//}


Driver::Driver()
{
  // load parameters
  driver_pars_ = new driverpars::DriverPars();
  bd_pars_ = new bdpars::BDPars();

  // one BDState object per core
  //bd_state_ = std::vector<BDState>(bd_pars_->NumCores(), BDState(bd_pars_, driver_pars_));
  for (unsigned int i = 0; i < bd_pars_->NumCores(); i++) {
    bd_state_.push_back(BDState(bd_pars_, driver_pars_));
  }

  // initialize buffers
  enc_buf_in_ = new MutexBuffer<EncInput>(driver_pars_->Get(driverpars::enc_buf_in_capacity));
  enc_buf_out_ = new MutexBuffer<EncOutput>(driver_pars_->Get(driverpars::enc_buf_out_capacity));
  dec_buf_in_ = new MutexBuffer<DecInput>(driver_pars_->Get(driverpars::dec_buf_in_capacity));

  for (unsigned int i = 0; i < bd_pars_->FunnelRoutes()->size(); i++) {
    MutexBuffer<DecOutput> * buf_ptr = new MutexBuffer<DecOutput>(driver_pars_->Get(driverpars::dec_buf_out_capacity));
    dec_bufs_out_.push_back(buf_ptr);
  }

  // initialize Encoder and Decoder
  enc_ = new Encoder(
      bd_pars_,
      enc_buf_in_, 
      enc_buf_out_, 
      driver_pars_->Get(driverpars::enc_chunk_size), 
      driver_pars_->Get(driverpars::enc_timeout_us)
  );

  dec_ = new Decoder(
      bd_pars_, 
      dec_buf_in_, 
      dec_bufs_out_, 
      driver_pars_->Get(driverpars::dec_chunk_size), 
      driver_pars_->Get(driverpars::dec_timeout_us)
  );

  // initialize Comm
  if (driver_pars_->Get(driverpars::comm_type) == driverpars::soft) {

    comm_ = new comm::CommSoft(
        *(driver_pars_->Get(driverpars::soft_comm_in_fname)),
        *(driver_pars_->Get(driverpars::soft_comm_out_fname)),
        dec_buf_in_,
        enc_buf_out_
    );

  } else if (driver_pars_->Get(driverpars::comm_type) == driverpars::libUSB) {
    assert(false && "libUSB Comm is not implemented");
  } else {
    assert(false && "unhandled comm_type");
  }
}


Driver::~Driver()
{
  delete driver_pars_;
  delete bd_pars_;
  delete enc_buf_in_;
  delete enc_buf_out_;
  delete dec_buf_in_;
  for (MutexBuffer<DecOutput> * it : dec_bufs_out_) {
    delete it;
  }
  delete enc_;
  delete dec_;
  delete comm_;
}


void Driver::testcall(const std::string& msg)
{
    std::cout << msg << std::endl;
}


void Driver::Start()
{
  // start all worker threads
  enc_->Start();
  dec_->Start();
  comm_->StartStreaming();
}

void Driver::Stop()
{
  enc_->Stop();
  dec_->Stop();
  comm_->StopStreaming();
}

/// Set toggle traffic_en only, keep dump_en the same, returns previous traffic_en.
/// If register state has not been set, dump_en -> 0
bool Driver::SetToggleTraffic(unsigned int core_id, bdpars::RegId reg_id, bool en) 
{
  bool traffic_en, dump_en, reg_valid;
  std::tie(traffic_en, dump_en, reg_valid) = bd_state_[core_id].GetToggle(reg_id);
  if ((en != traffic_en) || !reg_valid) {
    SetToggle(core_id, reg_id, en, dump_en & reg_valid);
  }
  return traffic_en;
}

/// Set toggle dump_en only, keep traffic_en the same, returns previous dump_en.
/// If register state has not been set, traffic_en -> 0
bool Driver::SetToggleDump(unsigned int core_id, bdpars::RegId reg_id, bool en)
{
  bool traffic_en, dump_en, reg_valid;
  std::tie(traffic_en, dump_en, reg_valid) = bd_state_[core_id].GetToggle(reg_id);
  if ((en != dump_en) || !reg_valid) {
    SetToggle(core_id, reg_id, traffic_en & reg_valid, en);
  }
  return dump_en;
}

/// Turn on tag traffic in datapath (also calls Start/KillSpikes)
void Driver::SetTagTrafficState(unsigned int core_id, bool en)
{
  for (auto& it : kTrafficRegs) {
    SetToggleTraffic(core_id, it, en);
  }
}


/// Turn on spike outputs for all neurons
void Driver::SetSpikeTrafficState(unsigned int core_id, bool en)
{
  SetToggleTraffic(core_id, bdpars::NEURON_DUMP_TOGGLE, en); 
}


/// Turn on spike outputs for all neurons
void Driver::SetSpikeDumpState(unsigned int core_id, bool en)
{
  SetToggleDump(core_id, bdpars::NEURON_DUMP_TOGGLE, en);
}


void Driver::PauseTraffic(unsigned int core_id) 
{
  assert(last_traffic_state_[core_id].size() == 0 && "called PauseTraffic twice before calling ResumeTraffic");
  last_traffic_state_[core_id] = {};
  for (auto& reg_id : kTrafficRegs) {
    bool last_state = SetToggleTraffic(core_id, reg_id, false);
    last_traffic_state_[core_id].push_back(last_state);
  }
  bd_state_[core_id].WaitForTrafficOff();
}

void Driver::ResumeTraffic(unsigned int core_id)
{
  assert(last_traffic_state_[core_id].size() > 0 && "called ResumeTraffic before calling PauseTraffic");
  unsigned int i = 0;
  for (auto& reg_id : kTrafficRegs) {
    SetToggleTraffic(core_id, reg_id, last_traffic_state_[core_id][i]);
  }
  last_traffic_state_[core_id] = {};
}


void Driver::SetDACValue(unsigned int core_id, bdpars::DACSignalId signal_id, unsigned int value)
{
  bdpars::RegId DAC_reg_id = bd_pars_->DACSignalIdToDACRegisterId(signal_id);

  // look up state of connection to ADC XXX
  bool DAC_to_ADC_conn_curr_state = false;

  FieldValues field_vals = {{bdpars::DAC_VALUE, value}, {bdpars::DAC_TO_ADC_CONN, DAC_to_ADC_conn_curr_state}};
  SetRegister(core_id, DAC_reg_id, field_vals);
}


void Driver::SetDACtoADCConnectionState(unsigned int core_id, bdpars::DACSignalId dac_signal_id, bool en)
{
  assert(false && "not implemented");
}


void DisconnectDACsfromADC(unsigned int core_id)
{
  assert(false && "not implemented");
}


/// Set large/small current scale for either ADC
void Driver::SetADCScale(unsigned int core_id, bool adc_id, const std::string & small_or_large)
{
  assert(false && "not implemented");
}


/// Turn ADC output on
void Driver::SetADCTrafficState(unsigned int core_id, bool en)
{
  assert(false && "not implemented");
}


void Driver::SetPAT(
    unsigned int core_id, 
    const std::vector<PATData> & data, ///< data to program
    unsigned int start_addr  ///< PAT memory address to start programming from, default 0
)
{
  bd_state_[core_id].SetPAT(start_addr, data);
  
  // pack data fields
  std::vector<uint64_t> data_fields = PackWords(*(bd_pars_->Word(bdpars::PAT)), DataToFieldVValues(data));

  // encapsulate according to RW format
  std::vector<uint64_t> prog_words = PackRWProgWords(*(bd_pars_->Word(bdpars::PAT_WRITE)), data_fields, start_addr);

  // transmit words
  PauseTraffic(core_id);
  SendToHorn(core_id, bd_pars_->HornLeafIdFor(bdpars::PAT), prog_words);
  ResumeTraffic(core_id);
}


void Driver::SetTAT(
    unsigned int core_id, 
    bool TAT_idx,                      ///< which TAT to program, 0 or 1
    const std::vector<TATData> & data, ///< data to program
    unsigned int start_addr            ///< PAT memory address to start programming from, default 0
)
{

  // TAT_idx -> TAT_id
  bdpars::MemId TAT_id;
  if (TAT_idx == 0) {
    bd_state_[core_id].SetTAT0(start_addr, data);
    TAT_id = bdpars::TAT0;
  } else if (TAT_idx == 1) {
    bd_state_[core_id].SetTAT1(start_addr, data);
    TAT_id = bdpars::TAT1;
  } else {
    assert(false && "TAT_idx must == 0 or 1");
  }
  
  // pack data fields
  std::vector<FieldValues> field_vals = DataToFieldVValues(data);
  std::vector<uint64_t> data_fields;
  for (unsigned int i = 0; i < field_vals.size(); i++) {
    unsigned int word_type = data[i].type; // XXX this is a little janky, we just iterated through data
    data_fields.push_back(PackWord(*(bd_pars_->Word(TAT_id, word_type)), field_vals[i]));
  }

  // encapsulate data fields according to RIWI format
  std::vector<uint64_t> prog_words = PackRIWIProgWords(
      *(bd_pars_->Word(bdpars::TAT_SET_ADDRESS)), 
      *(bd_pars_->Word(bdpars::TAT_WRITE_INCREMENT)), 
      data_fields,
      start_addr);

  // transmit data
  PauseTraffic(core_id);
  SendToHorn(core_id, bd_pars_->HornLeafIdFor(TAT_id), prog_words);
  ResumeTraffic(core_id);
}


void Driver::SetAM(
    unsigned int core_id,
    const std::vector<AMData> & data, ///< data to program
    unsigned int start_addr           ///< PAT memory address to start programming from, default 0
)
{
  bd_state_[core_id].SetAM(start_addr, data);
 
  // pack data fields
  std::vector<uint64_t> data_fields = PackWords(*(bd_pars_->Word(bdpars::AM)), DataToFieldVValues(data));

  // encapsulate according to RMW format
  std::vector<uint64_t> prog_words = PackRMWProgWords(
      *(bd_pars_->Word(bdpars::AM_SET_ADDRESS)), 
      *(bd_pars_->Word(bdpars::AM_READ_WRITE)), 
      *(bd_pars_->Word(bdpars::AM_INCREMENT)), 
      data_fields,
      start_addr);

  // encapsulate AM/MM
  std::vector<uint64_t> prog_words_encapsulated = PackAMMMWord(bdpars::AM, prog_words);

  // transmit to horn
  PauseTraffic(core_id);
  SendToHorn(core_id, bd_pars_->HornLeafIdFor(bdpars::AM), prog_words_encapsulated);
  ResumeTraffic(core_id);
}


void Driver::SetMM(
    unsigned int core_id,
    const std::vector<MMData> & data, ///< data to program
    unsigned int start_addr           ///< PAT memory address to start programming from, default 0
)
{
  bd_state_[core_id].SetMM(start_addr, data);

  // pack data fields
  std::vector<uint64_t> data_fields = PackWords(*(bd_pars_->Word(bdpars::MM)), DataToFieldVValues(data));

  // encapsulate according to RIWI format
  std::vector<uint64_t> prog_words = PackRIWIProgWords(
      *(bd_pars_->Word(bdpars::MM_SET_ADDRESS)), 
      *(bd_pars_->Word(bdpars::MM_WRITE_INCREMENT)), 
      data_fields,
      start_addr);

  // encapsulate AM/MM
  std::vector<uint64_t> prog_words_encapsulated = PackAMMMWord(bdpars::MM, prog_words);

  // transmit to horn
  PauseTraffic(core_id);
  SendToHorn(core_id, bd_pars_->HornLeafIdFor(bdpars::MM), prog_words_encapsulated);
  ResumeTraffic(core_id);
}


std::vector<PATData> Driver::DumpPAT(unsigned int core_id)
{
  // make dump words
  unsigned int PAT_size = bd_pars_->Size(bdpars::PAT);
  std::vector<uint64_t> dump_words = PackRWDumpWords(*(bd_pars_->Word(bdpars::PAT_READ)), 0, PAT_size);

  // transmit dump words, then block until all PAT words have been received
  // XXX if something goes terribly wrong and not all the words come back, this will hang
  PauseTraffic(core_id);
  SendToHorn(core_id, bd_pars_->HornLeafIdFor(bdpars::PAT), dump_words);
  std::vector<uint64_t> payloads = RecvFromFunnel(bdpars::DUMP_PAT, core_id, PAT_size);
  ResumeTraffic(core_id);

  // unpack payload field of DecOutput according to pat word format
  FieldVValues fields = UnpackWords(*bd_pars_->Word(bdpars::PAT), payloads);
  std::vector<PATData> PAT_contents = FieldVValuesToPATData(fields);

  return PAT_contents;
}


std::vector<uint64_t> Driver::PackRWProgWords(
    const bdpars::WordStructure & word_struct, 
    const std::vector<uint64_t> & payload, 
    unsigned int start_addr) const
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
    const bdpars::WordStructure & word_struct, 
    unsigned int start_addr,
    unsigned int end_addr) const
// XXX should vectorize, use PackWords
{
  std::vector<uint64_t> retval;
  for (unsigned int addr = start_addr; addr < end_addr; addr++) {
    retval.push_back(PackWord(word_struct, {{bdpars::ADDRESS, static_cast<uint64_t>(addr)}}));
  }
  return retval;
}


std::vector<uint64_t> Driver::PackRIWIProgWords(
    const bdpars::WordStructure & addr_word_struct, 
    const bdpars::WordStructure & write_word_struct, 
    const std::vector<uint64_t> & payload, 
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
    const bdpars::WordStructure & addr_word_struct, 
    const bdpars::WordStructure & read_word_struct, 
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
    const bdpars::WordStructure & addr_word_struct, 
    const bdpars::WordStructure & write_word_struct, 
    const bdpars::WordStructure & incr_word_struct,
    const std::vector<uint64_t> & payload, 
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
    retval.push_back(PackWord(incr_word_struct, {})); // only fixed value of increment op for this word
  }

  return retval;
}


std::vector<uint64_t> Driver::PackAMMMWord(bdpars::MemId AM_or_MM, const std::vector<uint64_t> & payload_data) const
// XXX should vectorize, use PackWords
{
  // the reason for a stop bit type is a bit of a hack
  // see BDModel, Process() needs to be able to differentiate between AM/MM
  // collapsed FVVs
  const bdpars::WordStructure * AM_MM_encapsulation;
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


void Driver::SendSpikes(const std::vector<SynSpike> & spikes)
{
  // XXX this circumvents going through the normal set of calls:
  // no FieldVValues/PackWords and no SerializeWords
  
  // We look up the width of the synapse and sign fields,
  // but we are hardcoding the order
  const bdpars::WordStructure * spike_word_struct = bd_pars_->Word(bdpars::INPUT_SPIKES);
  unsigned int widths[2] = {spike_word_struct->at(0).second, spike_word_struct->at(1).second};

  // we can at least assert that the line we wrote above is consistent with BDPars
  // if BDPars were to change, we want to know about it
  assert(spike_word_struct->at(0).first == bdpars::SYNAPSE_SIGN);
  assert(spike_word_struct->at(1).first == bdpars::SYNAPSE_ADDRESS);

  // get reference to enc_buf_in_'s memory
  EncInput * write_to = enc_buf_in_->LockBack(spikes.size());   
  for (unsigned int i = 0; i < spikes.size(); i++) {

    // XXX time? need to figure out what to do in the FPGA
    // probably need to insert delay events here
    
    uint32_t sign_and_synapse_id[2] = {static_cast<uint32_t>(SignedValToSignBit(spikes[i].sign)), spikes[i].synapse_id};
    uint32_t payload = Pack32(sign_and_synapse_id, widths, 2);

    unsigned int leaf_id_as_uint = bd_pars_->HornIdx(bd_pars_->HornLeafIdFor(bdpars::INPUT_SPIKES));

    // write to memory pointed to by LockBack()
    write_to[i] = {spikes[i].core_id, leaf_id_as_uint, payload};
  }
  // unlock
  enc_buf_in_->UnlockBack();
}


std::vector<NrnSpike> Driver::RecvSpikes(unsigned int max_to_recv)
{
  unsigned int buf_idx = bd_pars_->FunnelIdx(bd_pars_->FunnelLeafIdFor(bdpars::OUTPUT_SPIKES));
  std::vector<DecOutput> dec_out = dec_bufs_out_[buf_idx]->PopVect(max_to_recv, driver_pars_->Get(driverpars::RecvSpikes_timeout_us));

  std::vector<NrnSpike> retval;
  for (auto& el : dec_out) {
    // spikes don't have any data fields, payload == nrn addr
    retval.push_back({el.time_epoch, el.core_id, el.payload});
  }
  return retval;
}


std::pair<std::vector<uint32_t>, unsigned int> Driver::SerializeWordsToLeaf(const std::vector<uint64_t> & inputs, bdpars::HornLeafId leaf_id) const
{
  unsigned int input_width = bd_pars_->Width(leaf_id);
  unsigned int serialization = bd_pars_->Serialization(leaf_id);

  return SerializeWords<uint64_t, uint32_t>(inputs, input_width, serialization);
}

std::pair<std::vector<uint64_t>, std::vector<uint32_t> > Driver::DeserializeWordsFromLeaf(const std::vector<uint32_t> & inputs, bdpars::FunnelLeafId leaf_id) const
{
  unsigned int deserialization = bd_pars_->Serialization(leaf_id); 
  unsigned int deserialized_width = bd_pars_->Width(leaf_id);

  return DeserializeWords<uint32_t, uint64_t>(inputs, deserialized_width, deserialization);
}


void Driver::SendToHorn(
    unsigned int core_id, 
    bdpars::HornLeafId leaf_id,
    const std::vector<uint64_t> & payload) 
{
  // do serialization 
  std::vector<uint32_t> serialized_words;
  unsigned int serialized_width;
  std::tie(serialized_words, serialized_width) = SerializeWordsToLeaf(payload, leaf_id);

  // Encoder doesn't know about the enums, cast leaf_id as a uint
  unsigned int leaf_id_as_uint = static_cast<unsigned int>(leaf_id); 

  // package into EncInput
  std::vector<EncInput> enc_inputs;
  for (auto& serialized_word : serialized_words) {
    enc_inputs.push_back({core_id, leaf_id_as_uint, serialized_word});
  }
 
  enc_buf_in_->Push(enc_inputs);
}

std::vector<uint64_t> Driver::RecvFromFunnel(bdpars::FunnelLeafId leaf_id, unsigned int core_id, unsigned num_to_recv) 
{

  // This is a call of convenience. Often, we're interested not just in a particular
  // leaf's traffic, but also just the traffic from a particular core.
  //
  // This isn't implemented for the multi-core case. 
  // That's complicated, and probably needs some additional interfaces
  // in the MB to do some fancy locking, 
  // or we need to dynamically allocate MBs by core.

  // Decoder doesn't know about enums, cast leaf_id as uint
  // this is the index of the dec_bufs_out[] we want to pull from
  unsigned int leaf_id_as_uint = static_cast<unsigned int>(leaf_id); 
  MutexBuffer<DecOutput> * this_buf = dec_bufs_out_[leaf_id_as_uint];

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
    unsigned int num_to_pop = driver_pars_->Get(driverpars::dec_buf_out_capacity);
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

void Driver::SetRegister(unsigned int core_id, bdpars::RegId reg_id, const FieldValues & field_vals)
{
  const bdpars::WordStructure * reg_word_struct = bd_pars_->Word(reg_id);
  uint64_t payload = PackWord(*reg_word_struct, field_vals);

  // form vector of values to set BDState's reg state with, in WordStructure field order
  std::vector<unsigned int> field_vals_as_vect;
  for (auto& it : *reg_word_struct) {
    bdpars::WordFieldId field_id = it.first;
    field_vals_as_vect.push_back(field_vals.at(field_id));
  }
  bd_state_[core_id].SetReg(reg_id, field_vals_as_vect);

  SendToHorn(core_id, bd_pars_->HornLeafIdFor(reg_id), {payload});
}


void Driver::SetToggle(unsigned int core_id, bdpars::RegId toggle_id, bool traffic_en, bool dump_en)
{
  SetRegister(core_id, toggle_id, {{bdpars::TRAFFIC_ENABLE, traffic_en}, {bdpars::DUMP_ENABLE, dump_en}});
}

uint64_t Driver::ValueForSpecialFieldId(bdpars::WordFieldId field_id) 
{
  using namespace bdpars;

  switch (field_id)
  {
    case UNUSED :
    {
      return 0; // don't care
    }
    case FIXED_0 :
    {
      return 0;
    }
    case FIXED_1 :
    {
      return 1;
    }
    case FIXED_2 :
    {
      return 2;
    }
    case FIXED_3 :
    {
      return 3;
    }
    default :
    {
      assert(false && "no value supplied for a given field");
      return 0; // suppresses compiler warning
    }
  }
}

bool Driver::SpecialFieldValueMatches(bdpars::WordFieldId field_id, uint64_t val)
{
  using namespace bdpars;

  switch (field_id)
  {
    case UNUSED :
    {
      return true; // don't care (although probably should be 0)
    }
    case FIXED_0 :
    {
      return val == 0;
    }
    case FIXED_1 :
    {
      return val == 1;
    }
    case FIXED_2 :
    {
      return val == 2;
    }
    case FIXED_3 :
    {
      return val == 3;
    }
    default :
    {
      return true; // for normal fields, we don't care
    }
  }
}

uint64_t Driver::PackWord(const bdpars::WordStructure & word_struct, const FieldValues & field_values) 
{
  std::vector<unsigned int> widths_as_vect;
  std::vector<uint64_t> field_values_as_vect;
  for (auto& it : word_struct) {
    bdpars::WordFieldId field_id;
    unsigned int field_width;
    std::tie(field_id, field_width) = it;

    uint64_t field_value;
    if (field_values.count(field_id) > 0) {
      field_value = field_values.at(field_id);
    } else {
      field_value = ValueForSpecialFieldId(field_id);
    }
    
    widths_as_vect.push_back(field_width);
    field_values_as_vect.push_back(field_value);
  }

  return PackV64(field_values_as_vect, widths_as_vect);
}

// XXX can use this if I write an optimized version of Packwords()
//uint64_t Driver::PackWord(const bdpars::WordStructure & word_struct, const FieldValues & field_values) 
//{
//  FieldVValues fvv = FVasFVV(field_values);
//  std::vector<uint64_t> words = PackWords(word_struct, fvv);
//  assert(words.size() == 1);
//  return words.back();
//}

std::vector<uint64_t> Driver::PackWords(const bdpars::WordStructure & word_struct, const FieldVValues & field_values)
{
  // check that input is well-formed
  assert(word_struct.size() >= field_values.size() && "fewer fields in word_struct than field_values"); // can have some extra fields in FVV in some cases
  for (auto& it : field_values) {
    std::vector<uint64_t> vect = it.second;
    assert(vect.size() == field_values.begin()->second.size() && "FieldVValues vector length mismatch");
  }

  std::vector<uint64_t> output; 
  std::vector<FieldValues> vfv = FVVasVFV(field_values);
  for (auto& fv : vfv) {
    output.push_back(PackWord(word_struct, fv));
  }

  return output;
}

FieldValues Driver::UnpackWord(const bdpars::WordStructure & word_struct, uint64_t word)
{
  FieldVValues fvv = UnpackWords(word_struct, {word});
  std::vector<FieldValues> vfv = FVVasVFV(fvv);
  assert(vfv.size() == 1);
  return vfv.back();
}

FieldVValues Driver::UnpackWords(const bdpars::WordStructure & word_struct, std::vector<uint64_t> words)
{
  std::vector<unsigned int> widths_as_vect;
  for (auto& it : word_struct) {
    unsigned int field_width = it.second;
    widths_as_vect.push_back(field_width);
  }

  // set up retval, init vectors
  FieldVValues retval;
  for (auto& it : word_struct) {
    bdpars::WordFieldId field_id = it.first;
    retval[field_id] = {};
  }

  // fill in vectors
  for (unsigned int i = 0; i < words.size(); i++) {
    std::vector<uint64_t> vals = UnpackV64(words[i], widths_as_vect);
    unsigned int j = 0;
    for (auto& it : word_struct) {
      bdpars::WordFieldId field_id = it.first;

      // check if we're unpacking the right word, return empty object if not
      if (!SpecialFieldValueMatches(field_id, vals[j])) return {};

      retval[field_id].push_back(vals[j]);
      j++;
    }
  }

  return retval;
}

} // bddriver namespace
} // pystorm namespace
