#include "Driver.h"

#include <cassert>
#include <iostream>
#include <unordered_map>
#include <vector>

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

// XXX BEGIN "this should REALLY be in BDPars"
static const std::string OK_BITFILE = "OK_BITFILE.bit";
static const std::string OK_SERIAL = "";

std::map<bdpars::ConfigSomaID, std::vector<unsigned int>> Driver::config_soma_mem_ = {
  {bdpars::ConfigSomaID::GAIN_0          , {112 , 114 , 82 , 80 , 119 , 117 , 85 , 87 , 55 , 53 , 21 , 23 , 48 , 50 , 18 , 16}} ,
  {bdpars::ConfigSomaID::GAIN_1          , {104 , 97  , 65 , 72 , 111 , 102 , 70 , 79 , 47 , 38 , 6  , 15 , 40 , 33 , 1  , 8}}  ,
  {bdpars::ConfigSomaID::OFFSET_0        , {113 , 115 , 83 , 81 , 118 , 116 , 84 , 86 , 54 , 52 , 20 , 22 , 49 , 51 , 19 , 17}} ,
  {bdpars::ConfigSomaID::OFFSET_1        , {120 , 122 , 90 , 88 , 127 , 125 , 93 , 95 , 63 , 61 , 29 , 31 , 56 , 58 , 26 , 24}} ,
  {bdpars::ConfigSomaID::ENABLE          , {121 , 123 , 91 , 89 , 126 , 124 , 92 , 94 , 62 , 60 , 28 , 30 , 57 , 59 , 27 , 25}} ,
  {bdpars::ConfigSomaID::SUBTRACT_OFFSET , {96  , 106 , 74 , 64 , 103 , 109 , 77 , 71 , 39 , 45 , 13 , 7  , 32 , 42 , 10 , 0}}
};
  
std::map<bdpars::ConfigSynapseID, std::vector<unsigned int>> Driver::config_synapse_mem_ = {
  {bdpars::ConfigSynapseID::SYN_DISABLE , {75 , 76 , 12 , 11}} ,
  {bdpars::ConfigSynapseID::ADC_DISABLE , {67 , 68 , 4  , 3}}
};

std::map<bdpars::DiffusorCutLocationId, std::vector<unsigned int>> Driver::config_diff_cut_mem_ = {
  {bdpars::DiffusorCutLocationId::NORTH_LEFT  , {99}}  ,
  {bdpars::DiffusorCutLocationId::NORTH_RIGHT , {100}} ,
  {bdpars::DiffusorCutLocationId::WEST_TOP    , {107}} ,
  {bdpars::DiffusorCutLocationId::WEST_BOTTOM , {43}}  ,
};
// XXX END "this should REALLY be in BDPars"


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
  // bd_state_ = std::vector<BDState>(bd_pars_->NumCores, BDState(bd_pars_, driver_pars_));
  for (unsigned int i = 0; i < bd_pars_->NumCores; i++) {
    bd_state_.push_back(BDState(bd_pars_, driver_pars_));
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
      up_ep_deserializers_.insert({ep, VectorDeserializer<DecOutput>(D)});
    }
  }

  // initialize Encoder and Decoder
  enc_ = new Encoder(
      enc_buf_in_,
      enc_buf_out_,
      driver_pars_->Get(driverpars::ENC_TIMEOUT_US));

  dec_ = new Decoder(
      dec_buf_in_,
      dec_bufs_out_,
      driver_pars_->Get(driverpars::DEC_TIMEOUT_US));

  // initialize Comm
#ifdef BD_COMM_TYPE_SOFT
    comm_ = new comm::CommSoft(
        *(driver_pars_->Get(driverpars::SOFT_COMM_IN_FNAME)),
        *(driver_pars_->Get(driverpars::SOFT_COMM_OUT_FNAME)),
        dec_buf_in_,
        enc_buf_out_);
#elif BD_COMM_TYPE_USB
    assert(false && "libUSB Comm is not implemented");
#elif BD_COMM_TYPE_MODEL
    comm_ = nullptr;
#elif BD_COMM_TYPE_OPALKELLY
    comm_ = new comm::CommOK(dec_buf_in_, enc_buf_out_);
#else
    assert(false && "unhandled comm_type");
#endif

}

Driver::~Driver() {
  delete driver_pars_;
  delete bd_pars_;
  delete enc_buf_in_;
  delete enc_buf_out_;
  delete dec_buf_in_;
  for (auto& it : dec_bufs_out_) {
    delete it.second;
  }
  delete enc_;
  delete dec_;
  delete comm_;
}

void Driver::InitBD() {
#if BD_COMM_TYPE_OPALKELLY
    // Initialize Opal Kelly Board
    static_cast<comm::CommOK*>(comm_)->Init(OK_BITFILE, OK_SERIAL);
#endif

  for (unsigned int i = 0; i < bd_pars_->NumCores; i++) {
    // turn off traffic
    SetTagTrafficState(i, false);
    SetSpikeTrafficState(i, false);

    // Set the memory delays
    for(auto& mem : {bdpars::BDMemId::AM, bdpars::BDMemId::MM, bdpars::BDMemId::FIFO_PG, bdpars::BDMemId::FIFO_DCT, bdpars::BDMemId::TAT0, bdpars::BDMemId::TAT1, bdpars::BDMemId::PAT}) {
      const unsigned delay_val = 0;
      SetMemoryDelay(i, mem, delay_val, delay_val);
    }

    // init the FIFO
    InitFIFO(i);

    // clear all memories (perhaps unecessary? takes extra time)
    SetMem(i , bdpars::BDMemId::PAT  , std::vector<BDWord>(bd_pars_->mem_info_.at(bdpars::BDMemId::PAT).size  , 0) , 0);
    SetMem(i , bdpars::BDMemId::TAT0 , std::vector<BDWord>(bd_pars_->mem_info_.at(bdpars::BDMemId::TAT0).size , 0) , 0);
    SetMem(i , bdpars::BDMemId::TAT1 , std::vector<BDWord>(bd_pars_->mem_info_.at(bdpars::BDMemId::TAT1).size , 0) , 0);
    SetMem(i , bdpars::BDMemId::MM   , std::vector<BDWord>(bd_pars_->mem_info_.at(bdpars::BDMemId::MM).size   , 0) , 0);
    SetMem(i , bdpars::BDMemId::AM   , std::vector<BDWord>(bd_pars_->mem_info_.at(bdpars::BDMemId::AM).size   , 0) , 0);

    // XXX other stuff to do?
  }
}

void Driver::InitFIFO(unsigned int core_id) {
  // turn traffic off around FIFO (just kill everything to hijack the traffic drain timer in BDState)
  PauseTraffic(core_id);

  // make FIFO_HT head = tail (doesn't matter what you send)
  SendToEP(core_id, bdpars::BDHornEP::INIT_FIFO_HT, {0});

  // send all tag values to DCT FIF0 to dirty them so they flush
  std::vector<BDWord> all_tag_vals;
  for (unsigned int i = 0; i < bd_pars_->mem_info_.at(bdpars::BDMemId::FIFO_DCT).size; i++) {
    all_tag_vals.push_back(PackWord<FIFOInputTag>({{FIFOInputTag::TAG, i}}));
  }
  SendToEP(core_id, bdpars::BDHornEP::INIT_FIFO_DCT, all_tag_vals);

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
bool Driver::SetToggleTraffic(unsigned int core_id, bdpars::BDHornEP reg_id, bool en) {
  bool traffic_en, dump_en, reg_valid;
  std::tie(traffic_en, dump_en, reg_valid) = bd_state_[core_id].GetToggle(reg_id);
  if ((en != traffic_en) || !reg_valid) {
    SetToggle(core_id, reg_id, en, dump_en & reg_valid);
  }
  return traffic_en;
}

/// Set toggle dump_en only, keep traffic_en the same, returns previous dump_en.
/// If register state has not been set, traffic_en -> 0
bool Driver::SetToggleDump(unsigned int core_id, bdpars::BDHornEP reg_id, bool en) {
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
  SetToggleTraffic(core_id, bdpars::BDHornEP::NEURON_DUMP_TOGGLE, en);
}

/// Turn on spike outputs for all neurons
void Driver::SetSpikeDumpState(unsigned int core_id, bool en) {
  SetToggleDump(core_id, bdpars::BDHornEP::NEURON_DUMP_TOGGLE, en);
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


void Driver::SetMemoryDelay(unsigned int core_id, bdpars::BDMemId mem_id, unsigned int read_value, unsigned int write_value) {
  BDWord word = PackWord<DelayWord>({{DelayWord::READ_DELAY, read_value},
                                           {DelayWord::WRITE_DELAY, write_value}});
  SetBDRegister(core_id, bd_pars_->mem_info_.at(mem_id).delay_reg, word);
}

void Driver::SetPreFIFODumpState(unsigned int core_id, bool dump_en) {
  SetToggleDump(core_id, bdpars::BDHornEP::TOGGLE_PRE_FIFO, dump_en);
}

void Driver::SetPostFIFODumpState(unsigned int core_id, bool dump_en) {
  SetToggleDump(core_id, bdpars::BDHornEP::TOGGLE_POST_FIFO0, dump_en);
  SetToggleDump(core_id, bdpars::BDHornEP::TOGGLE_POST_FIFO1, dump_en);
}

std::vector<BDWord> Driver::GetPreFIFODump(unsigned int core_id) {
  return RecvFromEP(core_id, bdpars::BDFunnelEP::DUMP_PRE_FIFO).first;
}

std::pair<std::vector<BDWord>, std::vector<BDWord> > Driver::GetPostFIFODump(unsigned int core_id) {
  std::vector<BDWord> tags0, tags1;

  while (tags0.size() == 0 && tags1.size() == 0) {
    tags0 = RecvFromEP(core_id, bdpars::BDFunnelEP::DUMP_POST_FIFO0, 1000).first;
    tags1 = RecvFromEP(core_id, bdpars::BDFunnelEP::DUMP_POST_FIFO1, 1000).first;
  }

  return std::make_pair(tags0, tags1);
}

std::pair<unsigned int, unsigned int> Driver::GetFIFOOverflowCounts(unsigned int core_id) {
  std::vector<BDWord> ovflw0 = RecvFromEP(core_id, bdpars::BDFunnelEP::OVFLW0).first;
  std::vector<BDWord> ovflw1 = RecvFromEP(core_id, bdpars::BDFunnelEP::OVFLW1).first;
  return {ovflw0.size(), ovflw1.size()};
}

void Driver::SetDACValue(unsigned int core_id, bdpars::BDHornEP signal_id, unsigned int value) {
  assert(false && "not implemented");  // fix the ADC connection state thing below

  // look up state of connection to ADC XXX
  bool DAC_to_ADC_conn_curr_state = false;

  BDWord word = PackWord<DACWord>({{DACWord::DAC_VALUE, value}, {DACWord::DAC_TO_ADC_CONN, DAC_to_ADC_conn_curr_state}});
  SetBDRegister(core_id, signal_id, word);
}

void Driver::SetDACtoADCConnectionState(unsigned int core_id, bdpars::BDHornEP dac_signal_id, bool en) {
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
                       std::map<U, std::vector<unsigned int>> config_map,
                       U config_type,
                       unsigned int config_value) {
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

    PauseTraffic(core_id);
    SendToEP(core_id, bdpars::BDHornEP::NEURON_CONFIG, config_word);
    ResumeTraffic(core_id);
  }

////////////////////////////////////////////////////////////////////////////////
// Soma controls
////////////////////////////////////////////////////////////////////////////////

void Driver::SetSomaEnableStatus(unsigned int core_id, unsigned int soma_id,
                                 bdpars::SomaStatusId status) {
    unsigned int bit_val = (status == bdpars::SomaStatusId::DISABLED ? 0 : 1);
    SetSomaConfigMemory(core_id, soma_id, bdpars::ConfigSomaID::ENABLE, bit_val);
}

void Driver::SetSomaGain(unsigned int core_id, unsigned int soma_id,
                         bdpars::SomaGainId gain) {
    unsigned int g1, g0;
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
    unsigned int bit_val = (offset_sign == bdpars::SomaOffsetSignId::POSITIVE ? 0 : 1);
    SetSomaConfigMemory(core_id, soma_id, bdpars::ConfigSomaID::SUBTRACT_OFFSET, bit_val);
}

void Driver::SetSomaOffsetMultiplier(unsigned int core_id, unsigned int soma_id,
                                     bdpars::SomaOffsetMultiplierId offset_multiplier) {
    unsigned int b1, b0;
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
    unsigned int bit_val = (status == bdpars::SynapseStatusId::ENABLED ? 0 : 1);
    SetSynapseConfigMemory(core_id, synapse_id, bdpars::ConfigSynapseID::SYN_DISABLE, bit_val);
}

void Driver::SetSynapseADCStatus(unsigned int core_id, unsigned int synapse_id,
                                 bdpars::SynapseStatusId status) {
    unsigned int bit_val = (status == bdpars::SynapseStatusId::ENABLED ? 0 : 1);
    SetSynapseConfigMemory(core_id, synapse_id, bdpars::ConfigSynapseID::ADC_DISABLE, bit_val);
}

////////////////////////////////////////////////////////////////////////////////
// Diffusor controls
////////////////////////////////////////////////////////////////////////////////
void Driver::SetDiffusorCutStatus(unsigned int core_id,
                                  unsigned int tile_id,
                                  bdpars::DiffusorCutLocationId cut_id,
                                  bdpars::DiffusorCutStatusId status) {
    unsigned int bit_val = (status == bdpars::DiffusorCutStatusId::CLOSE ? 0 : 1);
    SetDiffusorConfigMemory(core_id, tile_id, cut_id, bit_val);
}

void Driver::SetDiffusorAllCutsStatus(unsigned int core_id,
                                      unsigned int tile_id,
                                      bdpars::DiffusorCutStatusId status) {
    unsigned int bit_val = (status == bdpars::DiffusorCutStatusId::CLOSE ? 0 : 1);

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
  if (mem_id == bdpars::BDMemId::AM) { // if we're programming the AM, we're also dumping the AM, need to sink what comes back
    bdpars::BDFunnelEP funnel_ep = bd_pars_->mem_info_.at(mem_id).dump_leaf;
    
    unsigned int mem_size = bd_pars_->mem_info_.at(mem_id).size;

    // XXX this might discard remainders
    unsigned int n_recvd = 0;
    while (n_recvd < data.size()) {
      std::pair<std::vector<BDWord>, std::vector<BDTime>> recvd = RecvFromEP(core_id, funnel_ep);
      n_recvd += recvd.first.size();
    }
    if (n_recvd > mem_size) {
      cout << "WARNING! LOST SOME AM WORDS" << endl;
    }
  }
  ResumeTraffic(core_id);
}


std::vector<BDWord> Driver::DumpMem(unsigned int core_id, bdpars::BDMemId mem_id) {

  // make dump words
  unsigned int mem_size = bd_pars_->mem_info_.at(mem_id).size;

  std::vector<BDWord> encapsulated_words;
  if (mem_id == bdpars::BDMemId::PAT) {
    encapsulated_words = PackRWDumpWords<PATRead>(0, mem_size);
  } else if (mem_id == bdpars::BDMemId::TAT0 || mem_id == bdpars::BDMemId::TAT1) {
    encapsulated_words = PackRIWIDumpWords<TATSetAddress, TATReadIncrement>(0, mem_size);
  } else if (mem_id == bdpars::BDMemId::MM) {
    encapsulated_words = PackRIWIDumpWords<MMSetAddress, MMReadIncrement>(0, mem_size);
  } else if (mem_id == bdpars::BDMemId::AM) {
    // a little tricky, reprogramming is the same as dump
    // need to write back whatever is currently in memory
    const std::vector<BDWord> *curr_data = bd_state_.at(core_id).GetMem(bdpars::BDMemId::AM);
    encapsulated_words = PackRMWProgWords<AMSetAddress, AMReadWrite, AMIncrement>(*curr_data, 0);
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
  bdpars::BDFunnelEP funnel_ep = bd_pars_->mem_info_.at(mem_id).dump_leaf;
  PauseTraffic(core_id);
  SendToEP(core_id, horn_ep, encapsulated_words);

  // XXX this might discard remainders
  std::vector<BDWord> payloads;
  while (payloads.size() < mem_size) {
    std::pair<std::vector<BDWord>, std::vector<BDTime>> recvd = RecvFromEP(core_id, funnel_ep);
    payloads.insert(payloads.end(), recvd.first.begin(), recvd.first.end());
  }
  if (payloads.size() > mem_size) {
    cout << "WARNING! LOST SOME MEM(BDMemId enum=" << int(mem_id) << ") WORDS" << endl;
  }
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

void Driver::SendSpikes(unsigned int core_id, const std::vector<BDWord>& spikes, const std::vector<BDTime> times) {
  assert(spikes.size() == times.size());

  // do something with times
  SendToEP(core_id, bdpars::BDHornEP::NEURON_INJECT, spikes, times);
}

void Driver::SendTags(unsigned int core_id, const std::vector<BDWord>& tags, const std::vector<BDTime> times) {
  assert(tags.size() == times.size());

  // do something with times
  SendToEP(core_id, bdpars::BDHornEP::RI, tags, times);
}


std::pair<std::vector<BDWord>,
          std::vector<BDTime>> Driver::RecvTags(unsigned int core_id) {

  typedef std::pair<std::vector<BDWord>, std::vector<BDTime>> RetType;

  RetType tat_tags; 
  RetType acc_tags;

  // XXX need to have a timeout, otherwise we can hang even when we have something to send
  while (tat_tags.first.size() == 0 && acc_tags.first.size() == 0) {
    tat_tags = RecvFromEP(core_id, bdpars::BDFunnelEP::RO_TAT, 1000);
    acc_tags = RecvFromEP(core_id, bdpars::BDFunnelEP::RO_ACC, 1000);
  }

  // concatenate
  tat_tags.first.insert(tat_tags.first.end()   , acc_tags.first.begin()  , acc_tags.first.end());
  tat_tags.second.insert(tat_tags.second.end() , acc_tags.second.begin() , acc_tags.second.end());

  return tat_tags;
}

void Driver::SendToEP(unsigned int core_id,
    uint8_t ep_code,
    const std::vector<BDWord>& payload,
    const std::vector<BDTime>& times) {

  std::unique_ptr<std::vector<EncInput>> serialized(new std::vector<EncInput>);

  const unsigned int FPGA_payload_width = FieldWidth(FPGAIO::PAYLOAD);
  const unsigned int ep_data_size = bd_pars_->Dn_EP_size_.at(ep_code);
  const unsigned int D = ep_data_size % FPGA_payload_width == 0 ? 
      ep_data_size / FPGA_payload_width 
    : ep_data_size / FPGA_payload_width + 1;

  if (D > 1) {
    // if the width of a single data object sent downstream ever
    // exceeds 64 bits, we may need to rethink this
    assert(D == 2); // only case to deal with for now
    unsigned int i = 0;
    for (auto& it : payload) {
      EncInput to_push[2];
      BDTime time = times.size() == 0 ? 0 : times.at(i); // time 0 goes in immediately

      // lsb first, msb second
      to_push[0].payload = GetField(it, TWOFPGAPAYLOADS::LSB);
      to_push[0].time = time;
      to_push[0].core_id = core_id;
      to_push[0].FPGA_ep_code = ep_code;

      to_push[1].payload = GetField(it, TWOFPGAPAYLOADS::MSB);
      to_push[1].time = time;
      to_push[1].core_id = core_id;
      to_push[1].FPGA_ep_code = ep_code;

      serialized->push_back(to_push[0]);
      serialized->push_back(to_push[1]);
      i++;
    }
  } else {
    unsigned int i = 0;
    for (auto& it : payload) {
      EncInput to_push;
      BDTime time = times.size() == 0 ? 0 : times.at(i); // time 0 goes in immediately
      
      to_push.payload = it;
      to_push.time = time;
      to_push.core_id = core_id;
      to_push.FPGA_ep_code = ep_code;

      serialized->push_back(to_push);
      i++;
    }
  }

  enc_buf_in_->Push(std::move(serialized));
}



std::pair<std::vector<BDWord>,
          std::vector<BDTime>>
  Driver::RecvFromEP(unsigned int core_id, uint8_t ep_code, unsigned int timeout_us) {

  // get data from buffer
  MutexBuffer<DecOutput>* this_buf = dec_bufs_out_.at(ep_code);
  std::unique_ptr<std::vector<DecOutput>> new_data = this_buf->Pop(timeout_us);

  std::vector<BDWord> words;
  std::vector<BDTime> times;

  // if there's a deserializer, use it
  if (up_ep_deserializers_.count(ep_code) > 0) {
    auto& deserializer = up_ep_deserializers_.at(ep_code);
    deserializer.NewInput(new_data.get());

    // read first word
    std::vector<DecOutput> deserialized; // continuosly write into here
    deserializer.GetOneOutput(&deserialized);
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
      times.push_back(time);

      deserializer.GetOneOutput(&deserialized);
    }
  } else {

    // otherwise, not much to do
    for (auto& it : *(new_data.get())) {
      uint32_t payload = it.payload;
      BDTime   time    = it.time; // take time on second word

      words.push_back(payload);
      times.push_back(time);
    }
  }

  return {words, times};
}

void Driver::SetBDRegister(unsigned int core_id, bdpars::BDHornEP reg_id, BDWord word) {
  // form vector of values to set BDState's reg state with, in WordStructure field order
  assert(bd_pars_->BDHornEPIsReg(reg_id));
  bd_state_[core_id].SetReg(reg_id, word);
  SendToEP(core_id, reg_id, {word});
}

void Driver::SetToggle(unsigned int core_id, bdpars::BDHornEP toggle_id, bool traffic_en, bool dump_en) {
  SetBDRegister(core_id, toggle_id, PackWord<ToggleWord>(
        {{ToggleWord::TRAFFIC_ENABLE, traffic_en}, {ToggleWord::DUMP_ENABLE, dump_en}}));
}

} // bddriver
} // pystorm
