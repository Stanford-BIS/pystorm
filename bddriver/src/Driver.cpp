#include <iostream>

#include "Driver.h"

#include "common/DriverTypes.h"
//#include "common/WordStream.h"
#include "common/BDPars.h"
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
  
  // initialize buffers
  enc_buf_in_ = new MutexBuffer<EncInput>(driver_pars_->Get("enc_buf_in_", "capacity"));
  enc_buf_out_ = new MutexBuffer<EncOutput>(driver_pars_->Get("enc_buf_out_", "capacity"));
  dec_buf_in_ = new MutexBuffer<DecInput>(driver_pars_->Get("dec_buf_in_", "capacity"));

  for (unsigned int i = 0; i < bd_pars_->FunnelRoutes()->size(); i++) {
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
void Driver::SetTagTrafficState(unsigned int core_id, bool en)
{
  // XXX look up dump state
  bool dump_state = false;
  for (auto& it : {TOGGLE_PRE_FIFO, TOGGLE_POST_FIFO0, TOGGLE_POST_FIFO1, NeuronDumpToggle}) {
    SetToggle(core_id, it, en, dump_state);
  }
}


/// Turn on spike outputs for all neurons
void Driver::SetSpikeTrafficState(unsigned int core_id, bool en)
{
  // XXX look up dump state
  bool dump_state = false;
  SetToggle(core_id, NeuronDumpToggle, en, dump_state);
}


/// Turn on spike outputs for all neurons
void Driver::SetSpikeDumpState(unsigned int core_id, bool en)
{
  // XXX look up traffic state
  bool traffic_state = false;
  SetToggle(core_id, NeuronDumpToggle, traffic_state, en);
}


void Driver::SetDACValue(unsigned int core_id, DACSignalId signal_id,  unsigned int value)
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
  // XXX ensure traffic is stopped, stop if necessary 
  
  std::vector<uint64_t> payload;
  for (auto& it : data) {

    FieldValues field_vals = 
        {{AM_address, it.AM_address},   
         {MM_address_lo, it.MM_address_lo}, 
         {MM_address_hi, it.MM_address_hi}};

    payload.push_back(PackWord(*(bd_pars_->Word(PAT)), field_vals));
  }

  // encapsulate according to RW format
  std::vector<uint64_t> prog_words = PackRWProgWords(*(bd_pars_->Word(PAT_write)), payload, start_addr);

  SendToHorn(core_id, bd_pars_->ProgHornId(PAT), prog_words);
}


std::vector<PATData> Driver::DumpPAT(unsigned int core_id)
{
  // XXX ensure traffic is stopped, stop if necessary 

  // make dump words
  unsigned int PAT_size = bd_pars_->Size(PAT);
  std::vector<uint64_t> dump_words = PackRWDumpWords(*(bd_pars_->Word(PAT_read)), 0, PAT_size);
  SendToHorn(core_id, bd_pars_->ProgHornId(PAT), dump_words);

  // wait to receive return values 
  // XXX CALLING THREAD IS BLOCKED UNTIL ALL WORDS RECEIVED
  // There's no max timeout for this call, currently.
  // If something goes wrong and all the words don't come back, you will hang.
  
  unsigned int total_recv = 0;
  DecOutput recv_vals[PAT_size];
  while (total_recv < PAT_size) {
    unsigned int funnel_idx = bd_pars_->FunnelIdx(bd_pars_->DumpFunnelId(PAT));
    unsigned int n_recv = dec_bufs_out_[funnel_idx]->Pop(&recv_vals[total_recv], PAT_size - total_recv, driver_pars_->Get("DumpPAT", "timeout_us"));
    total_recv += n_recv;
  }

  // unpack payload field of DecOutput according to pat word format
  std::vector<PATData> retval;
  for (unsigned int i = 0; i < PAT_size; i++) {
    DecOutput val = recv_vals[i];
    assert(val.core_id == core_id && "got PAT dump from wrong core");
    // XXX we throw the time on the floor, don't care about it
    FieldValues unpacked_vals = UnpackWord(*bd_pars_->Word(PAT), val.payload);
    retval.push_back(
        {static_cast<unsigned int>(unpacked_vals[AM_address]),
         static_cast<unsigned int>(unpacked_vals[MM_address_lo]), 
         static_cast<unsigned int>(unpacked_vals[MM_address_hi])}
    );
  }
  return retval;
}


void Driver::SetTAT(
    unsigned int core_id, 
    bool TAT_idx,                      ///< which TAT to program, 0 or 1
    const std::vector<TATData> & data, ///< data to program
    unsigned int start_addr            ///< PAT memory address to start programming from, default 0
)
{
  // XXX ensure traffic is stopped, stop if necessary 
  
  MemId TAT_id;
  if (TAT_idx == 0) {
    TAT_id = TAT0;
  } else if (TAT_idx == 1) {
    TAT_id = TAT1;
  } else {
    assert(false && "TAT_idx must == 0 or 1");
  }
 
  // pack data fields
  std::vector<uint64_t> payload;

  unsigned int even_odd_spike = 0;
  uint64_t last_synapse_addr = 0; // initialization value unused, suppresses compiler warning
  uint64_t last_synapse_sign = 0; // initialization value unused, suppresses compiler warning
  bool skip;
  for (auto& it : data) {

    skip = false;

    FieldValues field_vals;
    if (it.type == 0) { // address type
      field_vals = 
        {{AM_address, it.AM_address}, 
         {MM_address, it.MM_address},
         {stop,       it.stop}};

    } else if (it.type == 1) { // spike type
      if ((even_odd_spike % 2) == 0) {
        skip = true; // only push spike type every other element
      }

      field_vals = 
        {{synapse_address_0, last_synapse_addr},
         {synapse_sign_0,    SignedValToBit(last_synapse_sign)}, // -1/+1 -> 1/0
         {synapse_address_1, it.synapse_id}, 
         {synapse_sign_1,    SignedValToBit(it.synapse_sign)}, // -1/+1 -> 1/0
         {stop,              it.stop}};

      last_synapse_addr = it.synapse_id;
      last_synapse_sign = it.synapse_sign;
      even_odd_spike = (even_odd_spike + 1) % 2;

    } else if (it.type == 2) { // fanout type
        field_vals = 
          {{tag,          it.tag}, 
           {global_route, it.global_route},
           {stop,         it.stop}};
    }

    if (!skip) {
      payload.push_back(PackWord(*(bd_pars_->Word(TAT_id, it.type)), field_vals));
    }
  }

  // encapsulate according to RIWI format
  std::vector<uint64_t> prog_words = PackRIWIProgWords(
      *(bd_pars_->Word(TAT_set_address)), 
      *(bd_pars_->Word(TAT_write_increment)), 
      payload,
      start_addr);

  SendToHorn(core_id, bd_pars_->ProgHornId(TAT_id), prog_words);
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
        {{value,        0}, 
         {threshold,    it.threshold},
         {stop,         it.stop},
         {next_address, it.next_address}};

    payload.push_back(PackWord(*(bd_pars_->Word(AM)), field_vals));
  }

  // encapsulate according to RMW format
  std::vector<uint64_t> prog_words = PackRMWProgWords(
      *(bd_pars_->Word(AM_set_address)), 
      *(bd_pars_->Word(AM_read_write)), 
      *(bd_pars_->Word(AM_increment)), 
      payload, 
      start_addr);

  std::vector<uint64_t> prog_words_encapsulated = PackAMMMWord(AM, prog_words);

  SendToHorn(core_id, bd_pars_->ProgHornId(AM), prog_words_encapsulated);
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
        {{weight, it}};

    payload.push_back(PackWord(*(bd_pars_->Word(MM)), field_vals));
  }

  // encapsulate according to RIWI format
  std::vector<uint64_t> prog_words = PackRIWIProgWords(
      *(bd_pars_->Word(MM_set_address)), 
      *(bd_pars_->Word(MM_write_increment)), 
      payload, 
      start_addr);

  std::vector<uint64_t> prog_words_encapsulated = PackAMMMWord(MM, prog_words);

  SendToHorn(core_id, bd_pars_->ProgHornId(MM), prog_words_encapsulated);
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
    retval.push_back(PackWord(word_struct, {{address, addr}, {data, it}}));
    addr++;
  }

  return retval;
}

std::vector<uint64_t> Driver::PackRWDumpWords(
    const WordStructure & word_struct, 
    unsigned int start_addr,
    unsigned int end_addr) const
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
{
  const WordStructure * AM_MM_encapsulation;
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
    FieldValues field_values = {{synapse_sign, SignedValToBit(it.sign)}, {synapse_address, it.neuron_id}};
    payloads.push_back(PackWord(*(bd_pars_->Word(NeuronInject)), field_values));
  }

  // XXX figure out what to do with the delays

  SendToHorn(core_ids, std::vector<HornLeafId>(core_ids.size(), NeuronInject), payloads);
}


std::vector<Spike> Driver::RecvSpikes(unsigned int max_to_recv)
{
  unsigned int buf_idx = bd_pars_->FunnelIdx(NRNI);
  std::vector<DecOutput> dec_out = dec_bufs_out_[buf_idx]->PopVect(max_to_recv, driver_pars_->Get("RecvSpikes", "timeout_us"));

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
  SendToHorn(std::vector<unsigned int>(payload.size(), core_id), std::vector<HornLeafId>(payload.size(), leaf_id), payload);
}


void Driver::SetRegister(unsigned int core_id, RegId reg_id, const FieldValues & field_vals)
{
  const WordStructure * reg_word_struct = bd_pars_->Word(reg_id);
  uint64_t payload = PackWord(*reg_word_struct, field_vals);

  SendToHorn(core_id, bd_pars_->ProgHornId(reg_id), {payload});
}


void Driver::SetToggle(unsigned int core_id, RegId toggle_id, bool traffic_en, bool dump_en)
{
  SetRegister(core_id, toggle_id, {{traffic_enable, traffic_en}, {dump_enable, dump_en}});
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
    } else if (field_id == unused) { // user need not supply values for unused fields
      field_value = 0;
    } else if (field_id == FIXED_0) {
      field_value = 0;
    } else if (field_id == FIXED_1) {
      field_value = 1;
    } else if (field_id == FIXED_2) {
      field_value = 2;
    } else if (field_id == FIXED_3) {
      field_value = 3;
    } else {
      assert(false && "no value supplied for a given field");
    }
    
    widths_as_vect.push_back(field_width);
    field_values_as_vect.push_back(field_value);
  }

  return PackV64(field_values_as_vect, widths_as_vect);
}

//std::vector<uint64_t> Driver::PackWords(const WordStructure & word_struct, const WordStream & stream)
//{
//  // user must ensure that each vector element (which is an array) is ordered according to the word_struct
//  
//  // repackage word_struct field lens as array
//  assert(word_struct.size() == stream.NumFields() && "stream and word_struct don't match");
//
//  unsigned int n_fields = word_struct.size()
//  unsigned int len_arr[n_fields];
//  unsigned int i = 0;
//  for (auto& it : word_struct) {
//    len_arr[i++] = it.second;
//  }
//
//  std::vector<uint64_t> retval;
//  for (unsigned int i = 0; i < stream.Size(); i++) {
//    retval.push_back(Pack64(stream.Data(i), len_arr, n_fields));
//  }
//  return retval;
//
//}

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


uint64_t Driver::SignedValToBit(int sign) const {
  assert((sign == 1 || sign == -1) && "sign must be +1 or -1");
  return static_cast<uint64_t>((-1 * sign + 1) / 2);
}


} // bddriver namespace
} // pystorm namespace
