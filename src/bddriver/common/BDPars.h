#ifndef BDPARS_H
#define BDPARS_H

#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <typeinfo>
#include <typeindex>
#include <algorithm>

using std::cout;
using std::endl;

//////////////////////////////////////////////
// Downstream FPGA endpoints
//////////////////////////////////////////////

/// gcc 5 doesn't support enum class as map key.
/// Using EnumClassHash as the hash to the map
/// fixes the problem.
struct EnumClassHash
{
  template <typename T>
  std::size_t operator()(T t) const
  {
    return static_cast<std::size_t>(t);
  }
};

namespace pystorm {
namespace bddriver {
namespace bdpars {

// just a handle, the enum value doesn't represent anything
enum class DnEPType {
  BD_REG,
  BD_MEM,
  BD_INPUT,
  FPGA_REG,
  FPGA_CHANNEL,
  COUNT
};

//////////////////////////////////////////////
// BD Horn endpoints
// enum value is the code value

enum class BDHornEP {
  ADC,                // ADC small/large current enable, output enable
  DAC_DIFF_G,         // DIFF_G DAC bias value
  DAC_SYN_INH,        // SYN_INH DAC bias value
  DAC_SYN_PU,         // SYN_PU DAC bias value
  DAC_UNUSED,         // UNUSED (ghost DAC)
  DAC_DIFF_R,         // DIFF_R DAC bias value
  DAC_SOMA_OFFSET,    // SOMA_OFFSET DAC bias value
  DAC_SYN_LK,         // SYN_LK DAC bias value
  DAC_SYN_DC,         // SYN_DC DAC bias value
  DAC_SYN_PD,         // SYN_PD DAC bias value
  DAC_ADC_BIAS_2,     // ADC_BIAS_2 DAC bias value
  DAC_ADC_BIAS_1,     // ADC_BIAS_1 DAC bias value
  DAC_SOMA_REF,       // SOMA_REF DAC bias value
  DAC_SYN_EXC,        // SYN_EXC DAC bias value
  DELAY_DCTFIFO,      // FIFO:DCT delay line config
  DELAY_PGFIFO,       // FIFO:PG delay line config
  DELAY_TAT0,         // TAT 0 delay line config
  DELAY_TAT1,         // TAT 1 delay line config
  DELAY_PAT,          // PAT delay line config
  DELAY_MM,           // MM delay line config
  DELAY_AM,           // AM delay line config
  INIT_FIFO_DCT,      // inserts a tag into the DCT side of the FIFO
  INIT_FIFO_HT,       // trigger sets FIFO head/tail register to empty state
  NEURON_CONFIG,      // programming input for neuron array tile SRAM
  NEURON_DUMP_TOGGLE, // toggles data/dump traffic for neuron array output
  NEURON_INJECT,      // direct spike injection to neuron array
  PROG_AMMM,          // AM/MM programming/diagnostic port
  PROG_PAT,           // PAT programming/diagnostic port
  PROG_TAT0,          // TAT 0 programming/diagnostic port
  PROG_TAT1,          // TAT 1 programming/diagnostic port
  RI,                 // main tag input to FIFO
  TOGGLE_POST_FIFO0,  // toggles data/dump traffic for FIFO tag class 0 output
  TOGGLE_POST_FIFO1,  // toggles data/dump traffic for FIFO tag class 1 output
  TOGGLE_PRE_FIFO,    // toggles data/dump traffic for FIFO input
  COUNT
};

//////////////////////////////////////////////
// FPGA Registers 
// have different sizes
// hardcoded enum is the start reg address
// code is 128 + FPGARegEPId
enum class FPGARegEP {
  SF_FILTS_USED,           // highest SpikeFilter idx in use
  SF_INCREMENT_CONSTANT0,  // SpikeFilter increment per tag
  SF_INCREMENT_CONSTANT1,  // SpikeFilter increment per tag
  SF_DECAY_CONSTANT0,      // SpikeFilter decay per time unit
  SF_DECAY_CONSTANT1,      // SpikeFilter decay per time unit
  SG_GENS_USED,            // highest SpikeGenerator idx in use
  SG_GENS_EN0,             // bitmap of enables for each SpikeGenerator
  SG_GENS_EN1,             // bitmap of enables for each SpikeGenerator
  SG_GENS_EN2,             // bitmap of enables for each SpikeGenerator
  SG_GENS_EN3,             // bitmap of enables for each SpikeGenerator
  SG_GENS_EN4,             // bitmap of enables for each SpikeGenerator
  SG_GENS_EN5,             // bitmap of enables for each SpikeGenerator
  SG_GENS_EN6,             // bitmap of enables for each SpikeGenerator
  SG_GENS_EN7,             // bitmap of enables for each SpikeGenerator
  SG_GENS_EN8,             // bitmap of enables for each SpikeGenerator
  SG_GENS_EN9,             // bitmap of enables for each SpikeGenerator
  SG_GENS_EN10,            // bitmap of enables for each SpikeGenerator
  SG_GENS_EN11,            // bitmap of enables for each SpikeGenerator
  SG_GENS_EN12,            // bitmap of enables for each SpikeGenerator
  SG_GENS_EN13,            // bitmap of enables for each SpikeGenerator
  SG_GENS_EN14,            // bitmap of enables for each SpikeGenerator
  SG_GENS_EN15,            // bitmap of enables for each SpikeGenerator
  TM_UNIT_LEN,             // FPGA time resolution, units of 10ns
  TM_PC_TIME_ELAPSED0,     // Downstream heartbeat
  TM_PC_TIME_ELAPSED1,     // Downstream heartbeat
  TM_PC_TIME_ELAPSED2,     // Downstream heartbeat
  TM_PC_SEND_HB_UP_EVERY0, // Upstream heartbeart reporting period
  TM_PC_SEND_HB_UP_EVERY1, // Upstream heartbeart reporting period
  TM_PC_SEND_HB_UP_EVERY2, // Upstream heartbeart reporting period
  TM_PC_RESET_TIME,        // reset FPGA clock
  TS_REPORT_TAGS,          // forward tags to PC
  BD_RESET,                // bit 0 is pReset, bit 1 is sReset
  NOP,                     // sending to registers 32-63 is a NOP
  COUNT
};

//////////////////////////////////////////////
// FPGA Channels 
// use deserialization, 16 bits * number of transmissions
// code is 192 + FPGAChannelEPId
enum class FPGAChannelEP { 
  SG_PROGRAM_MEM,              // SG programming packet
  COUNT
};

//////////////////////////////////////////////
// FPGA upstream endpoints
//////////////////////////////////////////////

// just a handle, the enum value doesn't represent anything
enum class UpEPType {
  BD_OUTPUT,
  FPGA_OUTPUT,
  COUNT
};

//////////////////////////////////////////////
// BD funnel leaves

// the enum value is the upstream code
enum class BDFunnelEP {
  DUMP_AM,         // AM diagnostic read output
  DUMP_MM,         // MM diagnostic read output
  DUMP_PAT,        // PAT diagnostic read output
  DUMP_POST_FIFO0, // copy of tag class 0 traffic exiting FIFO
  DUMP_POST_FIFO1, // copy of tag class 1 traffic exiting FIFO
  DUMP_PRE_FIFO,   // copy of traffic entering FIFO
  DUMP_TAT0,       // TAT 0 diagnostic read output
  DUMP_TAT1,       // TAT 1 diagnostic read output
  NRNI,            // copy of traffic exiting neuron array
  OVFLW0,          // class 0 FIFO overflow warning
  OVFLW1,          // class 1 FIFO overflow warning
  RO_ACC,          // tag output from accumulator
  RO_TAT,          // tag output from TAT
  COUNT
};

//////////////////////////////////////////////
// FPGA outputs

// the enum value is the upstream code
enum class FPGAOutputEP {
  UPSTREAM_HB = 13, // Upstream report of FPGA clock
  SF_OUTPUT   = 14, // SpikeFilter outputs
  NOP         = 64, // NOP, inserted to pad output pipe
  COUNT       = 3   // XXX hardcoded, be careful
};


//////////////////////////////////////////////
// Misc identifiers
//////////////////////////////////////////////

//////////////////////////////////////////////
// Memory info
// memories have elaborate driver calls that are supported by this info

// just a handle, the enum value doesn't represent anything
enum class BDMemId {
  AM,
  MM,
  TAT0,
  TAT1,
  PAT,
  FIFO_DCT,
  FIFO_PG,
  COUNT
};

struct MemInfo {
  unsigned int size;
  BDHornEP prog_leaf;
  BDFunnelEP dump_leaf;
  BDHornEP delay_reg;
};

struct DACInfo {
  static constexpr float DAC_UNIT_CURRENT = 1e-12;
  static constexpr unsigned int DAC_MAX_COUNT = 1024;
  unsigned int scaling;
  unsigned int default_count;
};


//////////////////////////////////////////////
/// Neuron configuration options
enum class ConfigSomaID {
  GAIN_0,
  GAIN_1,
  OFFSET_0,
  OFFSET_1,
  ENABLE,
  SUBTRACT_OFFSET
};

enum class ConfigSynapseID {
  SYN_DISABLE,
  ADC_DISABLE
};

enum class SomaStatusId { DISABLED, ENABLED };

enum class SomaGainId { ONE_FOURTH, ONE_THIRD, ONE_HALF, ONE };

enum class SomaOffsetSignId { POSITIVE, NEGATIVE };

enum class SomaOffsetMultiplierId { ZERO, ONE, TWO, THREE };

enum class SynapseStatusId { ENABLED, DISABLED };

enum class DiffusorCutStatusId { CLOSE, OPEN };

enum class DiffusorCutLocationId { NORTH_LEFT, NORTH_RIGHT, WEST_TOP, WEST_BOTTOM };

/// BDPars holds all the nitty-gritty information about the BD hardware's parameters.
///
/// BDPars contains several array data members containing structs, keyed by enums.
/// The enums refer to particular hardware elements or concepts, such as the name of a memory,
/// register, or a particular type of programming word.
/// BDPars is fully public, but Driver only has a const reference.
class BDPars {

 public:

  // misc constants
  const unsigned int NumCores               = 1;
  const unsigned int DnEPFPGARegOffset      = 128;
  const unsigned int DnEPFPGANumReg         = 64;
  const unsigned int DnEPFPGAChannelOffset  = 192;
  const unsigned int DnEPFPGANumChan        = 2;
  
  const unsigned int DnEPFPGABitsPerReg     = 16;
  const unsigned int DnEPFPGABitsPerChannel = 16;

  const unsigned int DnWordsPerFrame        = 64; // FPGAIO words per USB frame
  const unsigned int DnTimeUnitsPerHB       = 10; // Send FPGA downstream heartbeat every <this many time units>

  // downstream endpoint info
  std::unordered_map<uint8_t , unsigned int> Dn_EP_size_;

  // upstream endpoint info
  std::unordered_map<uint8_t , unsigned int> Up_EP_size_;

  // memory info
  std::unordered_map<BDMemId, MemInfo, EnumClassHash> mem_info_;
  
  // DAC info
  std::unordered_map<BDHornEP, DACInfo, EnumClassHash> dac_info_;

  ///////////////////////////////
  // Neuron config stuff

  /// Config memory map
  /// Soma configuration bits for 16 Somas in a tile.
  static std::unordered_map<ConfigSomaID, std::vector<unsigned int>, EnumClassHash> config_soma_mem_;

  /// Synapse configuration bits for 4 Synapses in a tile.
  static std::unordered_map<ConfigSynapseID, std::vector<unsigned int>, EnumClassHash> config_synapse_mem_;

  /// Diffusor cut 'enable' memory config.
  /// Setting 1 cuts the diffusor at the location.
  static std::unordered_map<DiffusorCutLocationId, std::vector<unsigned int>, EnumClassHash> config_diff_cut_mem_;

  BDPars();

  // functions for info derived from other pars

  // downstream ep enums -> ep codes (FPGA addresses)
  inline uint8_t DnEPCodeFor(BDHornEP ep)      const { return static_cast<uint8_t>(ep); }
  inline uint8_t DnEPCodeFor(FPGARegEP ep)     const { return static_cast<uint8_t>(ep) + DnEPFPGARegOffset; }
  inline uint8_t DnEPCodeFor(FPGAChannelEP ep) const { return static_cast<uint8_t>(ep) + DnEPFPGAChannelOffset; }

  // up stream ep enums -> ep codes (FPGA addresses)
  inline uint8_t UpEPCodeFor(BDFunnelEP ep)   const { return static_cast<uint8_t>(ep); }
  inline uint8_t UpEPCodeFor(FPGAOutputEP ep) const { return static_cast<uint8_t>(ep); }

  // downstream ep code -> ep type
  inline bool DnEPCodeIsBDHornEP(uint8_t ep)      const { return ep < DnEPFPGARegOffset; }
  inline bool DnEPCodeIsFPGARegEP(uint8_t ep)     const { return ep >= DnEPFPGARegOffset && ep < DnEPFPGAChannelOffset; }
  inline bool DnEPCodeIsFPGAChannelEP(uint8_t ep) const { return ep >= DnEPFPGAChannelOffset; }

  // upstream ep code -> ep type
  inline bool UpEPCodeIsBDFunnelEP(uint8_t ep)   const { return ep < static_cast<uint8_t>(BDFunnelEP::COUNT); }
  inline bool UpEPCodeIsFPGAOutputEP(uint8_t ep) const { return ep >= static_cast<uint8_t>(BDFunnelEP::COUNT); }

  // get used up ep codes
  std::vector<uint8_t> GetUpEPs() const {
    std::vector<uint8_t> codes;
    for (auto& it : Up_EP_size_) {
      codes.push_back(it.first);
    }
    return codes;
  }

  // BDHornEP -> BD sub-type
  // XXX NOTE : this is only used by the model to aid parsing traffic
  bool BDHornEPIsInputStream(BDHornEP ep) const {
    const std::vector<BDHornEP> streams = {
      BDHornEP::NEURON_CONFIG, 
      BDHornEP::NEURON_INJECT, 
      BDHornEP::INIT_FIFO_DCT,
      BDHornEP::INIT_FIFO_HT,
      BDHornEP::RI};
    bool found = std::find(streams.begin(), streams.end(), ep) != streams.end();
    return found;
  }

  bool BDHornEPIsMem(BDHornEP ep) const {
    const std::vector<BDHornEP> mems = {
      BDHornEP::PROG_AMMM, 
      BDHornEP::PROG_PAT, 
      BDHornEP::PROG_TAT0, 
      BDHornEP::PROG_TAT1};
    bool found = std::find(mems.begin(), mems.end(), ep) != mems.end();
    return found;
  }
  
  bool BDHornEPIsReg(BDHornEP ep) const {
    // if it's not an input stream and it's not a memory, it's a reg
    return !BDHornEPIsMem(ep) && !BDHornEPIsInputStream(ep);
  }
  
  std::vector<BDHornEP> GetBDRegs() const {
    std::vector<BDHornEP> retval;
    for (auto& it : Dn_EP_size_) {
      uint8_t code = it.first;
      if (DnEPCodeIsBDHornEP(code)) {
        BDHornEP ep = static_cast<BDHornEP>(code);
        if (BDHornEPIsReg(ep)) {
          retval.push_back(ep);
        }
      }
    }
    return retval;
  }

};

} // bdpars
} // bddriver
} // pystorm

#endif
