#ifndef BDPARS_H
#define BDPARS_H

#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <array>
#include <typeinfo>
#include <typeindex>
#include <algorithm>
#include <cassert>

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
  ADC,                // 0:  ADC small/large current enable, output enable
  DAC_DIFF_G,         // 1:  DIFF_G DAC bias value
  DAC_SYN_INH,        // 2:  SYN_INH DAC bias value
  DAC_SYN_PU,         // 3:  SYN_PU DAC bias value
  DAC_UNUSED,         // 4:  UNUSED (ghost DAC)
  DAC_DIFF_R,         // 5:  DIFF_R DAC bias value
  DAC_SOMA_OFFSET,    // 6:  SOMA_OFFSET DAC bias value
  DAC_SYN_LK,         // 7:  SYN_LK DAC bias value
  DAC_SYN_DC,         // 8:  SYN_DC DAC bias value
  DAC_SYN_PD,         // 9:  SYN_PD DAC bias value
  DAC_ADC_BIAS_2,     // 10: ADC_BIAS_2 DAC bias value
  DAC_ADC_BIAS_1,     // 11: ADC_BIAS_1 DAC bias value
  DAC_SOMA_REF,       // 12: SOMA_REF DAC bias value
  DAC_SYN_EXC,        // 13: SYN_EXC DAC bias value
  DELAY_DCTFIFO,      // 14: FIFO:DCT delay line config
  DELAY_PGFIFO,       // 15: FIFO:PG delay line config
  DELAY_TAT0,         // 16: TAT 0 delay line config
  DELAY_TAT1,         // 17: TAT 1 delay line config
  DELAY_PAT,          // 18: PAT delay line config
  DELAY_MM,           // 19: MM delay line config
  DELAY_AM,           // 20: AM delay line config
  INIT_FIFO_DCT,      // 21: inserts a tag into the DCT side of the FIFO
  INIT_FIFO_HT,       // 22: trigger sets FIFO head/tail register to empty state
  NEURON_CONFIG,      // 23: programming input for neuron array tile SRAM
  NEURON_DUMP_TOGGLE, // 24: toggles data/dump traffic for neuron array output
  NEURON_INJECT,      // 25: direct spike injection to neuron array
  PROG_AMMM,          // 26: AM/MM programming/diagnostic port
  PROG_PAT,           // 27: PAT programming/diagnostic port
  PROG_TAT0,          // 28: TAT 0 programming/diagnostic port
  PROG_TAT1,          // 29: TAT 1 programming/diagnostic port
  RI,                 // 30: main tag input to FIFO
  TOGGLE_POST_FIFO0,  // 31: toggles data/dump traffic for FIFO tag class 0 output
  TOGGLE_POST_FIFO1,  // 32: toggles data/dump traffic for FIFO tag class 1 output
  TOGGLE_PRE_FIFO,    // 33: toggles data/dump traffic for FIFO input
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
  INVALID,         // something unexpected
  COUNT
};

//////////////////////////////////////////////
// FPGA outputs

// the enum value is the upstream code
enum class FPGAOutputEP {
  SF_OUTPUT       = 14,  // SpikeFilter outputs
  UPSTREAM_HB_LSB = 15,  // Upstream report of FPGA clock
  UPSTREAM_HB_MSB = 16,  // Upstream report of FPGA clock
  NOP             = 64,  // NOP, inserted to pad output pipe
  DS_QUEUE_CT     = 128, // first word of each block
  COUNT           = 4    // XXX hardcoded, be careful
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

  const unsigned int DnWordsPerFrame        = 256; // FPGAIO words per USB frame XXX same as OKComm's WRITE_SIZE*4, should get from here
  const unsigned int DnTimeUnitsPerHB       = 1; // Send FPGA downstream heartbeat every <this many time units>

  // downstream endpoint info
  std::unordered_map<uint8_t , unsigned int> Dn_EP_size_;

  // upstream endpoint info
  std::unordered_map<uint8_t , unsigned int> Up_EP_size_;

  // memory info
  std::unordered_map<BDMemId, MemInfo, EnumClassHash> mem_info_;
  
  // DAC info
  std::unordered_map<BDHornEP, DACInfo, EnumClassHash> dac_info_;

  // maps for AER address translation
  std::array<unsigned int, 4096> soma_xy_to_aer_;
  std::array<unsigned int, 4096> soma_aer_to_xy_;
  std::array<unsigned int, 1024> syn_xy_to_aer_;
  std::array<unsigned int, 1024> syn_aer_to_xy_;
  std::array<unsigned int, 256> mem_xy_to_aer_;
  std::array<unsigned int, 256> mem_aer_to_xy_;

  ///////////////////////////////
  // Neuron config stuff

  /// Config memory map
  /// Soma configuration bits for 16 Somas in a tile.
  static std::unordered_map<ConfigSomaID, std::vector<unsigned int>> config_soma_mem_;

  /// Synapse configuration bits for 4 Synapses in a tile.
  static std::unordered_map<ConfigSynapseID, std::vector<unsigned int>> config_synapse_mem_;

  /// Diffusor cut 'enable' memory config.
  /// Setting 1 cuts the diffusor at the location.
  static std::unordered_map<DiffusorCutLocationId, std::vector<unsigned int>> config_diff_cut_mem_;

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

  inline FPGARegEP GenIdxToSG_GENS_EN(unsigned int gen_idx) const {
    return static_cast<FPGARegEP>(static_cast<unsigned int>(FPGARegEP::SG_GENS_EN0) + gen_idx / 16);
  }

 private:

  // D is binary tree depth, not 4-ary tree depth, must be even
  template <int D>
  void InitAERMappers(std::array<unsigned int, (1<<D)> * xy_to_aer, std::array<unsigned int, (1<<D)> * aer_to_xy) {
    assert(D % 2 == 0); // D must be even
    for (unsigned int xy_idx = 0; xy_idx < (1<<D); xy_idx++) { // xy_idx is the xy flat xy_idx
      unsigned int array_edge_length = (1<<D/2);
      unsigned int x = xy_idx % array_edge_length;
      unsigned int y = xy_idx / array_edge_length;
      
      // ascend AER tree (lsbs -> msbs of xy), building up aer_idx
      uint16_t aer_idx = 0; // at most we need 12 bits for the AER addr
      for (unsigned int aer_node_idx = 0; aer_node_idx < D/2; aer_node_idx++) {
        // determine 2-bit AER code to give to each AER node
        unsigned int yx = ((y % 2) << 1) | x % 2;
        uint16_t aer_node_addr;
        if (yx == 0) {
          aer_node_addr = 0;
        } else if (yx == 1) {
          aer_node_addr = 1;
        } else if (yx == 2) {
          aer_node_addr = 3;
        } else if (yx == 3) {
          aer_node_addr = 2;
        } else {
          assert(false);
        }

        // write into aer_idx at correct locations (deepest nodes, last used, are msbs)
        unsigned int shift = 2 * aer_node_idx;
        aer_idx |= aer_node_addr << shift;

        // shift out x/y bits
        x = x >> 1;
        y = y >> 1;
      }

      // write results
      assert(xy_idx < 1<<D && "xy_idx too big");
      assert(aer_idx < 1<<D && "aer_idx too big");
      xy_to_aer->at(xy_idx) = aer_idx;
      aer_to_xy->at(aer_idx) = xy_idx;
      //cout << "xy: " << xy_idx << "aer: " << aer_idx << endl;

    }
  }


};

} // bdpars
} // bddriver
} // pystorm

#endif
