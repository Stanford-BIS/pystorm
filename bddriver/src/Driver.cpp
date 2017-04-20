#include <iostream>

#include "Driver.h"

#include "common/BDPars.h"
#include "common/DriverPars.h"
#include "common/HWLoc.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include "common/MutexBuffer.h"
#include "common/binary_util.h"

namespace pystorm
{
namespace bddriver
{


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
  
  // initialize buffers
  enc_buf_in_ = new MutexBuffer<EncInput>(driver_pars_->Get("enc_buf_in_", "capacity"));
  enc_buf_out_ = new MutexBuffer<EncOutput>(driver_pars_->Get("enc_buf_out_", "capacity"));
  dec_buf_in_ = new MutexBuffer<DecInput>(driver_pars_->Get("dec_buf_in_", "capacity"));

  for (unsigned int i = 0; i < bd_pars_->FunnelLeafNames().size(); i++) {
    MutexBuffer<DecOutput> * buf_ptr = new MutexBuffer<DecOutput>(driver_pars_->Get("dec_buf_out_", "capacity"));
    dec_bufs_out_.push_back(buf_ptr);
  }

  // initialize Encoder and Decoder
  enc_ = new Encoder(
      bd_pars_,
      enc_buf_in_, 
      enc_buf_out_, 
      driver_pars_->Get("enc_", "chunk_size"), 
      driver_pars_->Get("enc_", "timeout_us")
  );

  dec_ = new Decoder(
      bd_pars_, 
      dec_buf_in_, 
      dec_bufs_out_, 
      driver_pars_->Get("dec_", "chunk_size"), 
      driver_pars_->Get("dec_", "timeout_us")
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


/// Turn on tag traffic in datapath (also calls Start/KillSpikes)
void Driver::StartTraffic(unsigned int core_id)
{
  // XXX look up dump state
  bool dump_state = false;
  for (auto& it : {"TOGGLE_PRE_FIFO", "TOGGLE_POST_FIFO[0]", "TOGGLE_POST_FIFO[1]", "NeuronDumpToggle"}) {
    SetToggle(core_id, it, true, dump_state);
  }
}


/// Turn off tag traffic in datapath (also calls Start/KillSpikes)
void Driver::KillTraffic(unsigned int core_id)
{
  // XXX look up dump state
  bool dump_state = false;
  for (auto& it : {"TOGGLE_PRE_FIFO", "TOGGLE_POST_FIFO[0]", "TOGGLE_POST_FIFO[1]", "NeuronDumpToggle"}) {
    SetToggle(core_id, it, false, dump_state);
  }
}


/// Turn on spike outputs for all neurons
void Driver::StartSpikes(unsigned int core_id)
{
  // XXX look up dump state
  bool dump_state = false;
  SetToggle(core_id, "NeuronDumpToggle", true, dump_state);
}


/// Turn on spike output of a particular neuron
void Driver::StartSpikes(unsigned int core_id, unsigned int neuron_id) 
{
  // XXX program the tile memory
  assert(false && "not implemented");
}


/// Turn off spike outputs for all neurons
void Driver::KillSpikes(unsigned int core_id)
{
  // XXX look up dump state
  bool dump_state = false;
  SetToggle(core_id, "NeuronDumpToggle", false, dump_state);
}


/// Turn off spike output of a particular neuron
void Driver::KillSpikes(unsigned int core_id, unsigned int neuron_id)
{
  // XXX program the tile memory
  assert(false && "not implemented");
}


void Driver::SetDACValue(unsigned int core_id, const std::string & DAC_signal_name, unsigned int value)
{
  const std::string * DAC_register_name = bd_pars_->DACSignalNameToDACRegisterName(DAC_signal_name);

  // look up state of connection to ADC XXX
  bool DAC_to_ADC_conn = false;

  FieldValues field_vals = {{"DAC value", value}, {"DAC to ADC conn", DAC_to_ADC_conn}};
  SetRegister(core_id, *DAC_register_name, field_vals);
}

void Driver::ConnectDACtoADC(unsigned int core_id, const std::string & dac_signal_name)
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
void Driver::StartADCTraffic(unsigned int core_id)
{
  assert(false && "not implemented");
}


/// Turn ADC output off
void Driver::KillADCTraffic(unsigned int core_id)
{
  assert(false && "not implemented");
}


void Driver::SetPAT(
    unsigned int core_id, 
    const std::vector<PATData> & data, ///< data to program
    unsigned int start_addr            ///< PAT memory address to start programming from, default 0
)
{
  // XXX ensure traffic is stopped, stop if necessary 
  
  // pack data fields
  std::vector<uint64_t> payload;
  for (auto& it : data) {

    FieldValues field_vals = 
        {{"AM address", it.AM_addr}, 
         {"MM address low bits", it.MM_addr_low_bits}, 
         {"MM address base high bits", it.MM_addr_high_bits}};

    payload.push_back(PackWord(*(bd_pars_->Word("PAT")), field_vals));
  }

  // encapsulate according to RW format
  std::vector<uint64_t> prog_words = PackRWProgWords(*(bd_pars_->Word("PAT write")), payload, start_addr);

  SendToHorn(core_id, "PAT", prog_words);
}


void Driver::SetTAT(
    unsigned int core_id, 
    bool TAT_idx,                      ///< which TAT to program, 0 or 1
    const std::vector<TATData> & data, ///< data to program
    unsigned int start_addr            ///< PAT memory address to start programming from, default 0
)
{
  // XXX ensure traffic is stopped, stop if necessary 
 
  // pack data fields
  std::vector<uint64_t> payload;

  unsigned int even_odd_spike = 0;
  uint64_t last_synapse_addr;
  uint64_t last_synapse_sign;
  bool skip;
  for (auto& it : data) {

    skip = false;

    FieldValues field_vals;
    if (it.type == 0) { // address type
      field_vals = 
        {{"AM address", it.AM_addr}, 
         {"MM address", it.MM_addr},
         {"stop", it.stop}};

    } else if (it.type == 1) { // spike type
      if ((even_odd_spike % 2) == 0) {
        skip = true; // only push spike type every other element
      }

      field_vals = 
        {{"synapse id 0", last_synapse_addr},
         {"synapse sign 0", SignedValToBit(last_synapse_sign)}, // -1/+1 -> 1/0
         {"synapse id 1", it.tap_addr}, 
         {"synapse sign 1", SignedValToBit(it.tap_sign)}, // -1/+1 -> 1/0
         {"stop", it.stop}};

      last_synapse_addr = it.tap_addr;
      last_synapse_sign = it.tap_sign;
      even_odd_spike = (even_odd_spike + 1) % 2;

    } else if (it.type == 2) { // fanout type
        field_vals = 
          {{"tag", it.tag}, 
           {"global route", it.global_route},
           {"stop", it.stop}};
    }

    if (!skip) {
      payload.push_back(PackWord(*(bd_pars_->Word("TAT", it.type)), field_vals));
    }
  }

  // encapsulate according to RIWI format
  std::vector<uint64_t> prog_words = PackRIWIProgWords(
      *(bd_pars_->Word("TAT address")), 
      *(bd_pars_->Word("TAT write+increment")), 
      payload,
      start_addr);

  std::string TAT_name = "TAT[" + std::to_string(TAT_idx) + "]";
  SendToHorn(core_id, TAT_name, prog_words);
}


void Driver::SetAM(
    unsigned int core_id,
    const std::vector<AMData> & data, ///< data to program
    unsigned int start_addr           ///< PAT memory address to start programming from, default 0
)
{
  // XXX ensure traffic is stopped, stop if necessary 
 
  // pack data fields
  std::vector<uint64_t> payload;
  for (auto& it : data) {

    FieldValues field_vals = 
        {{"value", 0}, 
         {"threshold", it.threshold},
         {"stop", it.stop},
         {"next address", it.next_addr}};

    payload.push_back(PackWord(*(bd_pars_->Word("AM")), field_vals));
  }

  // encapsulate according to RMW format
  std::vector<uint64_t> prog_words = PackRMWProgWords(
      *(bd_pars_->Word("AM addr")), 
      *(bd_pars_->Word("AM read+write")), 
      *(bd_pars_->Word("AM increment")), 
      payload, 
      start_addr);

  std::vector<uint64_t> prog_words_encapsulated = PackAMMMWord("AM", prog_words);

  SendToHorn(core_id, "AMMM", prog_words_encapsulated);
}


void Driver::SetMM(
    unsigned int core_id,
    const std::vector<unsigned int> & data, ///< data to program
    unsigned int start_addr                 ///< PAT memory address to start programming from, default 0
)
{
  // XXX ensure traffic is stopped, stop if necessary 
 
  // pack data fields
  std::vector<uint64_t> payload;
  for (auto& it : data) {

    FieldValues field_vals = 
        {{"weight", it}};

    payload.push_back(PackWord(*(bd_pars_->Word("MM")), field_vals));
  }

  // encapsulate according to RIWI format
  std::vector<uint64_t> prog_words = PackRIWIProgWords(
      *(bd_pars_->Word("MM addr")), 
      *(bd_pars_->Word("MM write+increment")), 
      payload, 
      start_addr);

  std::vector<uint64_t> prog_words_encapsulated = PackAMMMWord("MM", prog_words);

  SendToHorn(core_id, "AMMM", prog_words_encapsulated);
}


std::vector<uint64_t> Driver::PackRWProgWords(
    const WordStructure & word_struct, 
    const std::vector<uint64_t> & payload, 
    unsigned int start_addr) const
// NOTE: word_struct is a parameter because memories with the same type may still have different field widths
{
  std::vector<uint64_t> retval;

  uint64_t addr = static_cast<uint64_t>(start_addr);
  for (auto& it : payload) {
    retval.push_back(PackWord(word_struct, {{"address", addr}, {"data", it}}));
    addr++;
  }

  return retval;
}


std::vector<uint64_t> Driver::PackRIWIProgWords(
    const WordStructure & addr_word_struct, 
    const WordStructure & write_word_struct, 
    const std::vector<uint64_t> & payload, 
    unsigned int start_addr) const
// NOTE: word_struct is a parameter because memories with the same type may still have different field widths
{

  std::vector<uint64_t> retval;

  // address word
  uint64_t addr = static_cast<uint64_t>(start_addr);
  retval.push_back(PackWord(addr_word_struct, {{"address", addr}}));

  for (auto& it : payload) {
    retval.push_back(PackWord(write_word_struct, {{"data", it}}));
  }

  return retval;
}


std::vector<uint64_t> Driver::PackRMWProgWords(
    const WordStructure & addr_word_struct, 
    const WordStructure & write_word_struct, 
    const WordStructure & incr_word_struct,
    const std::vector<uint64_t> & payload, 
    unsigned int start_addr) const
{
  std::vector<uint64_t> retval;

  // address word
  uint64_t addr = static_cast<uint64_t>(start_addr);
  retval.push_back(PackWord(addr_word_struct, {{"address", addr}}));

  for (auto& it : payload) {
    retval.push_back(PackWord(write_word_struct, {{"data", it}}));
    retval.push_back(PackWord(incr_word_struct, {})); // only fixed value of increment op for this word
  }

  return retval;
}


std::vector<uint64_t> Driver::PackAMMMWord(const std::string & AM_or_MM, const std::vector<uint64_t> & payload) const
{
  const WordStructure * AM_MM_encapsulation;
  if (AM_or_MM.compare("AM") == 0) {
    AM_MM_encapsulation = bd_pars_->Word("AM encapsulation");
  } else if (AM_or_MM.compare("MM") == 0) {
    AM_MM_encapsulation = bd_pars_->Word("MM encapsulation");
  } else {
    assert(false && "AM_or_MM must == 'AM' or 'MM'");
  }

  std::vector<uint64_t> retval;
  for (auto& it : payload) {

    uint64_t stop_bit;
    if (it == *payload.end()) {
      stop_bit = 1;
    } else {
      stop_bit = 0;
    }

    retval.push_back(PackWord(*AM_MM_encapsulation, {{"payload", it}, {"stop", stop_bit}}));
  }
  return retval;
}


void Driver::SendSpikes(
    const std::vector<Spike> & spikes,        ///< addresses of neurons to send spikes to
    const std::vector<unsigned int> & delays  ///< inter-spike intervals
)
{
  // XXX probably want to hardcode this for throughput
  assert(spikes.size() == delays.size() && "length of spike ids and delays must match") ;

  std::vector<uint64_t> payloads;
  std::vector<unsigned int> core_ids;
  payloads.reserve(spikes.size());
  for (auto& it : spikes) {
    core_ids.push_back(it.core_id);
    payloads.push_back(PackWord(*(bd_pars_->Word("NeuronInject")), {{"sign", SignedValToBit(it.sign)}}));
  }

  // XXX figure out what to do with the delays

  SendToHorn(core_ids, {"NeuronInject"}, payloads);
}


void Driver::SendToHorn(
    const std::vector<unsigned int> & core_id, 
    const std::vector<std::string> & leaf_name, 
    const std::vector<uint64_t> & payload) 
{

  // XXX DO SERIALIZATION HERE XXX
  
  assert(core_id.size() == leaf_name.size() && core_id.size() == payload.size() && "input lens must match");

  std::vector<EncInput> enc_inputs;
  for (unsigned int i = 0; i < payload.size(); i++) {
    HWLoc loc = {core_id[i], bd_pars_->HornIdx(leaf_name[i])};
    enc_inputs.push_back(std::make_pair(loc, static_cast<uint32_t>(payload[i])));
  }
 
  enc_buf_in_->Push(enc_inputs);
}


void Driver::SendToHorn(unsigned int core_id, const std::string & leaf_name, std::vector<uint64_t> payload) 
{
  SendToHorn({core_id}, {leaf_name}, payload);
}


void Driver::SetRegister(unsigned int core_id, const std::string & reg_name, const FieldValues & field_vals)
{
  const WordStructure * reg_word_struct = bd_pars_->Word(reg_name);
  uint64_t payload = PackWord(*reg_word_struct, field_vals);

  SendToHorn(core_id, reg_name, {payload});
}


void Driver::SetToggle(unsigned int core_id, const std::string & toggle_name, bool traffic_enable, bool dump_enable)
{
  SetRegister(core_id, toggle_name, {{"traffic_enable", traffic_enable}, {"dump_enable", dump_enable}});
}


uint64_t Driver::PackWord(const WordStructure & word_struct, const FieldValues & field_values) const
{
  std::vector<unsigned int> widths_as_vect;
  std::vector<uint64_t> field_values_as_vect;
  for (auto& it : word_struct) {
    std::string field_name = it.first;
    unsigned int field_width = it.second;

    uint64_t field_value;
    if (field_values.count(field_name) > 0) {
      field_value = field_values.at(field_name);
    } else if (field_name.compare("unused") == 0) { // user need not supply values for unused fields
      field_value = 0;
    } else if (field_name.substr(0,5).compare("FIXED") == 0) { // "FIXED=<X>" is the syntax for a fixed value
      auto eq_pos = field_name.find_first_of("=");
      field_value = std::stoul(field_name.substr(eq_pos + 1, std::string::npos)); // npos means go to end of string
    } else {
      assert(false && "no value supplied for a given field");
    }
    
    widths_as_vect.push_back(field_width);
    field_values_as_vect.push_back(field_value);
  }

  return PackV64(field_values_as_vect, widths_as_vect);
}


uint64_t Driver::SignedValToBit(int sign) const {
  assert((sign == 1 || sign == -1) && "sign must be +1 or -1");
  return static_cast<uint64_t>((-1 * sign + 1) / 2);
}


} // bddriver namespace
} // pystorm namespace
