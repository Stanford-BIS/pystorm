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

namespace pystorm {
namespace bddriver {


Driver& Driver::getInstance()
{
    // In C++11, if control from two threads occurs concurrently, execution
    // shall wait during static variable initialization, therefore, this is 
    // thread safe
    static Driver m_instance;
    return m_instance;
}


Driver::Driver()
{
  // load parameters
  driver_pars_ = new DriverPars();
  bd_pars_ = new BDPars();

  // one BDState object per core
  bd_state_ = std::vector<BDState>(bd_pars_->NumCores(), BDState(bd_pars_, driver_pars_));
  
  // initialize buffers
  enc_buf_in_ = new MutexBuffer<EncInput>(driver_pars_->Get(enc_buf_in_capacity));
  enc_buf_out_ = new MutexBuffer<EncOutput>(driver_pars_->Get(enc_buf_out_capacity));
  dec_buf_in_ = new MutexBuffer<DecInput>(driver_pars_->Get(dec_buf_in_capacity));

  for (unsigned int i = 0; i < bd_pars_->FunnelRoutes()->size(); i++) {
    MutexBuffer<DecOutput> * buf_ptr = new MutexBuffer<DecOutput>(driver_pars_->Get(dec_buf_out_capacity));
    dec_bufs_out_.push_back(buf_ptr);
  }

  // initialize Encoder and Decoder
  enc_ = new Encoder(
      bd_pars_,
      enc_buf_in_, 
      enc_buf_out_, 
      driver_pars_->Get(enc_chunk_size), 
      driver_pars_->Get(enc_timeout_us)
  );

  dec_ = new Decoder(
      bd_pars_, 
      dec_buf_in_, 
      dec_bufs_out_, 
      driver_pars_->Get(dec_chunk_size), 
      driver_pars_->Get(dec_timeout_us)
  );
}


Driver::~Driver()
{
  delete driver_pars_;
  delete bd_pars_;
  delete enc_buf_in_;
  delete enc_buf_out_;
  delete dec_buf_in_;
  for (auto& it : dec_bufs_out_) {
    delete it;
  }
  delete enc_;
  delete dec_;
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
}

/// Set toggle traffic_en only, keep dump_en the same, returns previous traffic_en.
/// If register state has not been set, dump_en -> 0
bool Driver::SetToggleTraffic(unsigned int core_id, RegId reg_id, bool en) 
{
  bool traffic_en, dump_en, reg_valid;
  std::tie(traffic_en, dump_en, reg_valid) = bd_state_[core_id].GetToggle(reg_id);
  if (en != traffic_en) {
    SetToggle(core_id, reg_id, en, dump_en & reg_valid);
  }
  return traffic_en;
}

/// Set toggle dump_en only, keep traffic_en the same, returns previous dump_en
/// If register state has not been set, traffic_en -> 0
bool Driver::SetToggleDump(unsigned int core_id, RegId reg_id, bool en)
{
  bool traffic_en, dump_en, reg_valid;
  std::tie(traffic_en, dump_en, reg_valid) = bd_state_[core_id].GetToggle(reg_id);
  if (en != dump_en) {
    SetToggle(core_id, reg_id, traffic_en & reg_valid, en);
  }
  return dump_en;
}

/// Turn on tag traffic in datapath (also calls Start/KillSpikes)
void Driver::SetTagTrafficState(unsigned int core_id, bool en)
{
  for (auto& it : {TOGGLE_PRE_FIFO, TOGGLE_POST_FIFO0, TOGGLE_POST_FIFO1, NeuronDumpToggle}) {
    SetToggleTraffic(core_id, it, en);
  }
}


/// Turn on spike outputs for all neurons
void Driver::SetSpikeTrafficState(unsigned int core_id, bool en)
{
  SetToggleTraffic(core_id, NeuronDumpToggle, en);
}


/// Turn on spike outputs for all neurons
void Driver::SetSpikeDumpState(unsigned int core_id, bool en)
{
  SetToggleDump(core_id, NeuronDumpToggle, en);
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


void Driver::SetDACValue(unsigned int core_id, DACSignalId signal_id, unsigned int value)
{
  RegId DAC_reg_id = bd_pars_->DACSignalIdToDACRegisterId(signal_id);

  // look up state of connection to ADC XXX
  bool DAC_to_ADC_conn_curr_state = false;

  FieldValues field_vals = {{DAC_value, value}, {DAC_to_ADC_conn, DAC_to_ADC_conn_curr_state}};
  SetRegister(core_id, DAC_reg_id, field_vals);
}


void Driver::SetDACtoADCConnectionState(unsigned int core_id, DACSignalId dac_signal_id, bool en)
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
  
  PauseTraffic(core_id);

  // pack data fields
  std::vector<uint64_t> data_fields = PackWords(*(bd_pars_->Word(PAT)), DataToFieldVValues(data));

  // encapsulate according to RW format
  std::vector<uint64_t> prog_words = PackRWProgWords(*(bd_pars_->Word(PAT_write)), data_fields, start_addr);

  // transmit words
  SendToHorn(core_id, bd_pars_->ProgHornId(PAT), prog_words);

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
  MemId TAT_id;
  if (TAT_idx == 0) {
    bd_state_[core_id].SetTAT0(start_addr, data);
    TAT_id = TAT0;
  } else if (TAT_idx == 1) {
    bd_state_[core_id].SetTAT1(start_addr, data);
    TAT_id = TAT1;
  } else {
    assert(false && "TAT_idx must == 0 or 1");
  }
  
  PauseTraffic(core_id);
  
  // pack data fields
  std::vector<FieldValues> field_vals = DataToFieldVValues(data);
  std::vector<uint64_t> data_fields;
  for (unsigned int i = 0; i < field_vals.size(); i++) {
    unsigned int word_type = data[i].type; // XXX this is a little janky, we just iterated through data
    data_fields.push_back(PackWord(*(bd_pars_->Word(TAT_id, word_type)), field_vals[i]));
  }

  // encapsulate data fields according to RIWI format
  std::vector<uint64_t> prog_words = PackRIWIProgWords(
      *(bd_pars_->Word(TAT_set_address)), 
      *(bd_pars_->Word(TAT_write_increment)), 
      data_fields,
      start_addr);

  // transmit data
  SendToHorn(core_id, bd_pars_->ProgHornId(TAT_id), prog_words);

  ResumeTraffic(core_id);
}


void Driver::SetAM(
    unsigned int core_id,
    const std::vector<AMData> & data, ///< data to program
    unsigned int start_addr           ///< PAT memory address to start programming from, default 0
)
{
  bd_state_[core_id].SetAM(start_addr, data);

  PauseTraffic(core_id);
 
  // pack data fields
  std::vector<uint64_t> data_fields = PackWords(*(bd_pars_->Word(AM)), DataToFieldVValues(data));

  // encapsulate according to RMW format
  std::vector<uint64_t> prog_words = PackRMWProgWords(
      *(bd_pars_->Word(AM_set_address)), 
      *(bd_pars_->Word(AM_read_write)), 
      *(bd_pars_->Word(AM_increment)), 
      data_fields,
      start_addr);

  // encapsulate AM/MM
  std::vector<uint64_t> prog_words_encapsulated = PackAMMMWord(AM, prog_words);

  // transmit to horn
  SendToHorn(core_id, bd_pars_->ProgHornId(AM), prog_words_encapsulated);

  ResumeTraffic(core_id);
}


void Driver::SetMM(
    unsigned int core_id,
    const std::vector<MMData> & data, ///< data to program
    unsigned int start_addr                 ///< PAT memory address to start programming from, default 0
)
{
  bd_state_[core_id].SetMM(start_addr, data);

  PauseTraffic(core_id);
 
  // pack data fields
  std::vector<uint64_t> data_fields = PackWords(*(bd_pars_->Word(MM)), DataToFieldVValues(data));

  // encapsulate according to RIWI format
  std::vector<uint64_t> prog_words = PackRIWIProgWords(
      *(bd_pars_->Word(MM_set_address)), 
      *(bd_pars_->Word(MM_write_increment)), 
      data_fields,
      start_addr);

  // encapsulate AM/MM
  std::vector<uint64_t> prog_words_encapsulated = PackAMMMWord(MM, prog_words);

  // transmit to horn
  SendToHorn(core_id, bd_pars_->ProgHornId(MM), prog_words_encapsulated);

  ResumeTraffic(core_id);
}


std::vector<PATData> Driver::DumpPAT(unsigned int core_id)
{
  PauseTraffic(core_id);

  // make dump words
  unsigned int PAT_size = bd_pars_->Size(PAT);
  std::vector<uint64_t> dump_words = PackRWDumpWords(*(bd_pars_->Word(PAT_read)), 0, PAT_size);

  // transmit dump words
  SendToHorn(core_id, bd_pars_->ProgHornId(PAT), dump_words);

  // wait to receive return values 
  // XXX CALLING THREAD IS BLOCKED UNTIL ALL WORDS RECEIVED
  // There's no max timeout for this call, currently.
  
  DecOutput recv_vals[PAT_size];
  unsigned int funnel_idx = bd_pars_->FunnelIdx(bd_pars_->DumpFunnelId(PAT));
  unsigned int timeout = driver_pars_->Get(DumpPAT_timeout_us);
  unsigned int n_recv = dec_bufs_out_[funnel_idx]->Pop(recv_vals, PAT_size, timeout);
  if (n_recv != PAT_size) {
    // XXX this should probably go to a warning log instead of stdio
    cout << "WARNING: DumpPAT(): didn't get all PAT values back in time" << endl;
  }

  // unpack payload field of DecOutput according to pat word format
  // first put payloads into a vector
  std::vector<uint64_t> payloads;
  for (unsigned int i = 0; i < n_recv; i++) {
    DecOutput val = recv_vals[i];
    assert(val.core_id == core_id && "got PAT dump from wrong core");
    // XXX we throw the time on the floor, don't care about it
    payloads.push_back(static_cast<uint64_t>(val.payload));
  }

  FieldVValues fields = UnpackWords(*bd_pars_->Word(PAT), payloads);

  ResumeTraffic(core_id);
  return FieldVValuesToPATData(fields);
}


std::vector<uint64_t> Driver::PackRWProgWords(
    const WordStructure & word_struct, 
    const std::vector<uint64_t> & payload, 
    unsigned int start_addr) const
// NOTE: word_struct is a parameter because memories with the same type may still have different field widths
// XXX should vectorize, use PackWords
{
  std::vector<uint64_t> retval;

  uint64_t addr = static_cast<uint64_t>(start_addr);
  for (auto& it : payload) {
    retval.push_back(PackWord(word_struct, {{address, addr}, {data, it}}));
    addr++;
  }

  return retval;
}


std::vector<uint64_t> Driver::PackRWDumpWords(
    const WordStructure & word_struct, 
    unsigned int start_addr,
    unsigned int end_addr) const
// XXX should vectorize, use PackWords
{
  std::vector<uint64_t> retval;
  for (unsigned int addr = start_addr; addr < end_addr; addr++) {
    retval.push_back(PackWord(word_struct, {{address, static_cast<uint64_t>(addr)}}));
  }
  return retval;
}


std::vector<uint64_t> Driver::PackRIWIProgWords(
    const WordStructure & addr_word_struct, 
    const WordStructure & write_word_struct, 
    const std::vector<uint64_t> & payload, 
    unsigned int start_addr) const
// NOTE: word_struct is a parameter because memories with the same type may still have different field widths
// XXX should vectorize, use PackWords
{

  std::vector<uint64_t> retval;

  // address word
  uint64_t addr = static_cast<uint64_t>(start_addr);
  retval.push_back(PackWord(addr_word_struct, {{address, addr}}));

  for (auto& it : payload) {
    retval.push_back(PackWord(write_word_struct, {{data, it}}));
  }

  return retval;
}


std::vector<uint64_t> Driver::PackRIWIDumpWords(
    const WordStructure & addr_word_struct, 
    const WordStructure & read_word_struct, 
    unsigned int start_addr,
    unsigned int end_addr) const
// XXX should vectorize, use PackWords
{
  std::vector<uint64_t> retval;
  retval.push_back(PackWord(addr_word_struct, {{address, static_cast<uint64_t>(start_addr)}}));
  for (unsigned int addr = start_addr; addr < end_addr; addr++) {
    retval.push_back(PackWord(read_word_struct, {}));
  }
  return retval;
}


std::vector<uint64_t> Driver::PackRMWProgWords(
    const WordStructure & addr_word_struct, 
    const WordStructure & write_word_struct, 
    const WordStructure & incr_word_struct,
    const std::vector<uint64_t> & payload, 
    unsigned int start_addr) const
// XXX should vectorize, use PackWords
{
  // RMWProgWord == RMWDumpWord
  
  std::vector<uint64_t> retval;

  // address word
  uint64_t addr = static_cast<uint64_t>(start_addr);
  retval.push_back(PackWord(addr_word_struct, {{address, addr}}));

  for (auto& it : payload) {
    retval.push_back(PackWord(write_word_struct, {{data, it}}));
    retval.push_back(PackWord(incr_word_struct, {})); // only fixed value of increment op for this word
  }

  return retval;
}


std::vector<uint64_t> Driver::PackAMMMWord(MemId AM_or_MM, const std::vector<uint64_t> & payload_data) const
// XXX should vectorize, use PackWords
{
  const WordStructure * AM_MM_encapsulation = nullptr; // assignment to nullptr suppresses compiler warning
  if (AM_or_MM == AM) {
    AM_MM_encapsulation = bd_pars_->Word(AM_encapsulation);
  } else if (AM_or_MM == MM) {
    AM_MM_encapsulation = bd_pars_->Word(MM_encapsulation);
  } else {
    assert(false && "AM_or_MM must == AM or MM");
  }

  std::vector<uint64_t> retval;
  for (auto& it : payload_data) {

    uint64_t stop_bit;
    if (it == *payload_data.end()) {
      stop_bit = 1;
    } else {
      stop_bit = 0;
    }

    uint64_t word = PackWord(*AM_MM_encapsulation, {{payload, it}, {stop, stop_bit}});
    retval.push_back(word);
  }
  return retval;
}


void Driver::SendSpikes(const std::vector<Spike> & spikes)
{
  // XXX probably want to hardcode this for throughput

  std::vector<uint64_t> payloads;
  std::vector<unsigned int> core_ids;
  payloads.reserve(spikes.size());
  for (auto& it : spikes) {
    core_ids.push_back(it.core_id);
    // XXX neuron_id != synapse id. Figure out how to resolve this... hopefully don't need input spike vs output spike type
    FieldValues field_values = {{synapse_sign, SignedValToSignBit(it.sign)}, {synapse_address, it.neuron_id}};
    payloads.push_back(PackWord(*(bd_pars_->Word(NeuronInject)), field_values));
  }

  // XXX figure out what to do with the delays

  SendToHorn(core_ids, std::vector<HornLeafId>(core_ids.size(), NeuronInject), payloads);
}


std::vector<Spike> Driver::RecvSpikes(unsigned int max_to_recv)
{
  unsigned int buf_idx = bd_pars_->FunnelIdx(NRNI);
  std::vector<DecOutput> dec_out = dec_bufs_out_[buf_idx]->PopVect(max_to_recv, driver_pars_->Get(RecvSpikes_timeout_us));

  std::vector<Spike> retval;
  for (auto& el : dec_out) {
    // spikes don't have any data fields, payload == nrn addr
    retval.push_back({el.time_epoch, el.core_id, el.payload, 1});
  }
  return retval;
}


void Driver::SendToHorn(
    const std::vector<unsigned int> & core_id, 
    const std::vector<HornLeafId> & leaf_id,
    const std::vector<uint64_t> & payload) 
{

  // XXX DO SERIALIZATION HERE XXX
  
  assert(core_id.size() == leaf_id.size() && core_id.size() == payload.size() && "input lens must match");
  
  std::vector<EncInput> enc_inputs;
  for (unsigned int i = 0; i < payload.size(); i++) {
    uint32_t payload_32 = static_cast<uint32_t>(payload[i]); // XXX won't need this once serialization is implemented
    unsigned int leaf_id_as_int = static_cast<unsigned int>(leaf_id[i]); // Encoder/decoder don't know about the enums
    enc_inputs.push_back({core_id[i], leaf_id_as_int, payload_32});
  }
 
  enc_buf_in_->Push(enc_inputs);
}

void Driver::SendToHorn(unsigned int core_id, HornLeafId leaf_id, std::vector<uint64_t> payload) 
{
  std::vector<unsigned int> core_ids = std::vector<unsigned int>(payload.size(), core_id);
  std::vector<HornLeafId> leaf_ids = std::vector<HornLeafId>(payload.size(), leaf_id);

  SendToHorn(core_ids, leaf_ids, payload);
}


void Driver::SetRegister(unsigned int core_id, RegId reg_id, const FieldValues & field_vals)
{
  const WordStructure * reg_word_struct = bd_pars_->Word(reg_id);
  uint64_t payload = PackWord(*reg_word_struct, field_vals);

  std::vector<unsigned int> field_vals_as_vect;
  for (auto& it : field_vals) {
    field_vals_as_vect.push_back(it.second);
  }

  bd_state_[core_id].SetReg(reg_id, field_vals_as_vect);
  SendToHorn(core_id, bd_pars_->ProgHornId(reg_id), {payload});
}


void Driver::SetToggle(unsigned int core_id, RegId toggle_id, bool traffic_en, bool dump_en)
{
  SetRegister(core_id, toggle_id, {{traffic_enable, traffic_en}, {dump_enable, dump_en}});
}

uint64_t Driver::ValueForSpecialFieldId(WordFieldId field_id) const {
  if (field_id == unused) { // user need not supply values for unused fields
    return 0;
  } else if (field_id == FIXED_0) {
    return 0;
  } else if (field_id == FIXED_1) {
    return 1;
  } else if (field_id == FIXED_2) {
    return 2;
  } else if (field_id == FIXED_3) {
    return 3;
  } else {
    assert(false && "no value supplied for a given field");
    return 0; // suppresses compiler warning
  }
}

uint64_t Driver::PackWord(const WordStructure & word_struct, const FieldValues & field_values) const
{
  std::vector<unsigned int> widths_as_vect;
  std::vector<uint64_t> field_values_as_vect;
  for (auto& it : word_struct) {
    WordFieldId field_id = it.first;
    unsigned int field_width = it.second;

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

std::vector<uint64_t> Driver::PackWords(const WordStructure & word_struct, const FieldVValues & field_values) const
{
  // check that input is well-formed
  assert(word_struct.size() == field_values.size() && "number of fields in word_struct and field_values");
  unsigned int n_fields = word_struct.size();
  unsigned int n_words = field_values.begin()->second.size();
  for (auto& it : field_values) {
    std::vector<uint64_t> vect = it.second;
    assert(vect.size() == n_words && "FieldVValues vector length mismatch");
  }

  // XXX not sure if it's better to rearrange first (which needs alloc), then Pack, or just iterate and pack
  // this is the rearrange then pack option
  uint64_t * vals = new uint64_t[n_words * n_fields];
  unsigned int i = 0;
  for (auto& it : word_struct) {
    WordFieldId field_id = it.first; // field id ~ i

    for (unsigned int j = 0; j < n_words; j++) {

      uint64_t field_value;
      if (field_values.count(field_id) > 0) {
        field_value = field_values.at(field_id).at(j);
      } else {
        field_value = ValueForSpecialFieldId(field_id);
      }

      vals[n_fields * j + i] = field_value;
    }
    i++;
  }

  // get the array of field lengths
  unsigned int * len_arr = new unsigned int[n_fields];
  i = 0;
  for (auto& it : word_struct) {
    len_arr[i++] = it.second;
  }

  // pack data from rearranged vals
  std::vector<uint64_t> retval;
  for (unsigned int i = 0; i < n_words; i++) {
    retval.push_back(Pack64(&vals[i*n_fields], len_arr, n_fields));
  }

  // clean up
  delete[] vals;
  delete[] len_arr;

  return retval;
}

FieldValues Driver::UnpackWord(const WordStructure & word_struct, uint64_t word) const
{
  std::vector<unsigned int> widths_as_vect;
  for (auto& it : word_struct) {
    unsigned int field_width = it.second;
    widths_as_vect.push_back(field_width);
  }

  std::vector<uint64_t> vals = UnpackV64(word, widths_as_vect);

  FieldValues retval;
  unsigned int i = 0;
  for (auto& it : word_struct) {
    WordFieldId field_id = it.first;
    retval[field_id] = vals[i];
    i++;
  }
  return retval;
}

FieldVValues Driver::UnpackWords(const WordStructure & word_struct, std::vector<uint64_t> words) const 
{
  std::vector<unsigned int> widths_as_vect;
  for (auto& it : word_struct) {
    unsigned int field_width = it.second;
    widths_as_vect.push_back(field_width);
  }

  // set up retval, init vectors
  FieldVValues retval;
  for (auto& it : word_struct) {
    WordFieldId field_id = it.first;
    retval[field_id] = {};
  }

  // fill in vectors
  for (unsigned int i = 0; i < words.size(); i++) {
    std::vector<uint64_t> vals = UnpackV64(words[i], widths_as_vect);
    unsigned int j = 0;
    for (auto& it : word_struct) {
      WordFieldId field_id = it.first;
      retval[field_id].push_back(vals[j]);
      j++;
    }
  }

  return retval;
}

} // bddriver namespace
} // pystorm namespace
