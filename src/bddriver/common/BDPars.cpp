#include "BDPars.h"

#include <string>
#include <unordered_map>
#include <array>

namespace pystorm {
namespace bddriver {
namespace bdpars {

constexpr unsigned int BDPars::NumNeurons;
constexpr unsigned int BDPars::NumSynapses;
constexpr unsigned int BDPars::NumCores;
constexpr unsigned int BDPars::TimingRoute;
constexpr unsigned int BDPars::DnEPFPGARegOffset;
constexpr unsigned int BDPars::DnEPFPGANumReg;
constexpr unsigned int BDPars::DnEPFPGAChannelOffset;
constexpr unsigned int BDPars::DnEPFPGANumChan;
constexpr unsigned int BDPars::DnEPFPGABitsPerReg;
constexpr unsigned int BDPars::DnEPFPGABitsPerChannel;
constexpr unsigned int BDPars::DnWordsPerFrame;
constexpr unsigned int BDPars::DnTimeUnitsPerHB;

// clang-format off

std::unordered_map<ConfigSomaID, std::vector<unsigned int>> BDPars::config_soma_mem_ = {
  {bdpars::ConfigSomaID::GAIN_0          , {112 , 114 , 82 , 80 , 119 , 117 , 85 , 87 , 55 , 53 , 21 , 23 , 48 , 50 , 18 , 16}} ,
  {bdpars::ConfigSomaID::GAIN_1          , {104 , 97  , 65 , 72 , 111 , 102 , 70 , 79 , 47 , 38 , 6  , 15 , 40 , 33 , 1  , 8}}  ,
  {bdpars::ConfigSomaID::OFFSET_0        , {113 , 115 , 83 , 81 , 118 , 116 , 84 , 86 , 54 , 52 , 20 , 22 , 49 , 51 , 19 , 17}} ,
  {bdpars::ConfigSomaID::OFFSET_1        , {120 , 122 , 90 , 88 , 127 , 125 , 93 , 95 , 63 , 61 , 29 , 31 , 56 , 58 , 26 , 24}} ,
  {bdpars::ConfigSomaID::ENABLE          , {121 , 123 , 91 , 89 , 126 , 124 , 92 , 94 , 62 , 60 , 28 , 30 , 57 , 59 , 27 , 25}} ,
  {bdpars::ConfigSomaID::SUBTRACT_OFFSET , {96  , 106 , 74 , 64 , 103 , 109 , 77 , 71 , 39 , 45 , 13 , 7  , 32 , 42 , 10 , 0}}
};

std::unordered_map<ConfigSynapseID, std::vector<unsigned int>> BDPars::config_synapse_mem_ = {
  {bdpars::ConfigSynapseID::SYN_DISABLE , {75 , 76 , 12 , 11}} ,
  {bdpars::ConfigSynapseID::ADC_DISABLE , {67 , 68 , 4  , 3}}
};

std::unordered_map<DiffusorCutLocationId, std::vector<unsigned int>> BDPars::config_diff_cut_mem_ = {
  {bdpars::DiffusorCutLocationId::NORTH_LEFT  , {99}}  ,
  {bdpars::DiffusorCutLocationId::NORTH_RIGHT , {100}} ,
  {bdpars::DiffusorCutLocationId::WEST_TOP    , {107}} ,
  {bdpars::DiffusorCutLocationId::WEST_BOTTOM , {43}}  ,
};

BDPars::BDPars() {
  //////////////////////////////////////////////////////
  // FPGA downstream endpoints
  // BD inputs, registers, and channels

  //////////////////////////////////////////////////////
  // BD inputs

  // serialization is 1 for most leaves
  for (unsigned int i = 0; i < static_cast<unsigned int>(BDHornEP::COUNT); i++) {
    Dn_EP_size_[DnEPCodeFor(static_cast<BDHornEP>(i))]        = 20; // don't care: shorter than 20b
  }
  // is 3 for some leaves
  Dn_EP_size_[DnEPCodeFor(BDHornEP::PROG_AMMM)]               = 42; 
  Dn_EP_size_[DnEPCodeFor(BDHornEP::PROG_PAT)]                = 27;
  Dn_EP_size_[DnEPCodeFor(BDHornEP::PROG_TAT0)]               = 31;
  Dn_EP_size_[DnEPCodeFor(BDHornEP::PROG_TAT1)]               = 31;

  //////////////////////////////////////////////////////
  // FPGA Registers

  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SF_FILTS_USED)]           = 9;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SF_INCREMENT_CONSTANT0)]  = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SF_INCREMENT_CONSTANT1)]  = 9; // 27 total
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SF_DECAY_CONSTANT0)]      = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SF_DECAY_CONSTANT1)]      = 9; // 27 total
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SG_GENS_USED)]            = 8;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SG_GENS_EN0)]             = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SG_GENS_EN1)]             = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SG_GENS_EN2)]             = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SG_GENS_EN3)]             = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SG_GENS_EN4)]             = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SG_GENS_EN5)]             = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SG_GENS_EN6)]             = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SG_GENS_EN7)]             = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SG_GENS_EN8)]             = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SG_GENS_EN9)]             = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SG_GENS_EN10)]            = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SG_GENS_EN11)]            = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SG_GENS_EN12)]            = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SG_GENS_EN13)]            = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SG_GENS_EN14)]            = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::SG_GENS_EN15)]            = 16; // 256 total
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::TM_UNIT_LEN)]             = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::TM_PC_TIME_ELAPSED0)]     = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::TM_PC_TIME_ELAPSED1)]     = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::TM_PC_TIME_ELAPSED2)]     = 16; // 48 total
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::TM_PC_SEND_HB_UP_EVERY0)] = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::TM_PC_SEND_HB_UP_EVERY1)] = 16;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::TM_PC_SEND_HB_UP_EVERY2)] = 16; // 48 total
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::TM_PC_RESET_TIME)]        = 1;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::TS_REPORT_TAGS)]          = 1;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::BD_RESET)]                = 2;
  Dn_EP_size_[DnEPCodeFor(FPGARegEP::NOP)]                     = 1;

  //////////////////////////////////////////////////////
  // FPGA Channels

  Dn_EP_size_[DnEPCodeFor(FPGAChannelEP::SG_PROGRAM_MEM)]     = 51;

  //////////////////////////////////////////////////////
  // FPGA upstream endpoints
  // BD outputs, FPGA outputs

  // serialization is 1 for most leaves
  for (unsigned int i = 0; i < static_cast<unsigned int>(BDFunnelEP::COUNT); i++) {
    Up_EP_size_[UpEPCodeFor(static_cast<BDFunnelEP>(i))]  = 20; // don't care, shorter than 20b
  }
  // serialization is 2 for a few others
  Up_EP_size_[UpEPCodeFor(BDFunnelEP::DUMP_AM)]           = 38;
  Up_EP_size_[UpEPCodeFor(BDFunnelEP::DUMP_TAT0)]         = 29;
  Up_EP_size_[UpEPCodeFor(BDFunnelEP::DUMP_TAT1)]         = 29;
  Up_EP_size_[UpEPCodeFor(BDFunnelEP::RO_ACC)]            = 28;
  Up_EP_size_[UpEPCodeFor(BDFunnelEP::RO_TAT)]            = 32;
  Up_EP_size_[UpEPCodeFor(BDFunnelEP::INVALID)]           = 34;

  //////////////////////////////////////////////////////
  // FPGA outputs

  Up_EP_size_[UpEPCodeFor(FPGAOutputEP::UPSTREAM_HB_LSB)] = 20;
  Up_EP_size_[UpEPCodeFor(FPGAOutputEP::UPSTREAM_HB_MSB)] = 20;
  Up_EP_size_[UpEPCodeFor(FPGAOutputEP::SF_OUTPUT)]       = 37;
  Up_EP_size_[UpEPCodeFor(FPGAOutputEP::NOP)]             = 20; // actually zero?
  Up_EP_size_[UpEPCodeFor(FPGAOutputEP::DS_QUEUE_CT)]     = 20;

  //////////////////////////////////////////////////////
  // memory info

  mem_info_[BDMemId::AM]       = {1024,    BDHornEP::PROG_AMMM, BDFunnelEP::DUMP_AM,   BDHornEP::DELAY_AM};
  mem_info_[BDMemId::MM]       = {64*1024, BDHornEP::PROG_AMMM, BDFunnelEP::DUMP_MM,   BDHornEP::DELAY_MM};
  mem_info_[BDMemId::TAT0]     = {1024,    BDHornEP::PROG_TAT0, BDFunnelEP::DUMP_TAT0, BDHornEP::DELAY_TAT0};
  mem_info_[BDMemId::TAT1]     = {1024,    BDHornEP::PROG_TAT1, BDFunnelEP::DUMP_TAT1, BDHornEP::DELAY_TAT1};
  mem_info_[BDMemId::PAT]      = {64,      BDHornEP::PROG_PAT,  BDFunnelEP::DUMP_PAT,  BDHornEP::DELAY_PAT};
  mem_info_[BDMemId::FIFO_DCT] = {2048,    BDHornEP::COUNT,     BDFunnelEP::COUNT,     BDHornEP::DELAY_DCTFIFO};
  mem_info_[BDMemId::FIFO_PG]  = {2048,    BDHornEP::COUNT,     BDFunnelEP::COUNT,     BDHornEP::DELAY_PGFIFO};

  // DAC info
  dac_info_[BDHornEP::DAC_ADC_BIAS_1]  = {1 , 512}; // roughly 1pA to 1nA
  dac_info_[BDHornEP::DAC_ADC_BIAS_2]  = {1 , 512}; // roughly 1pA to 1nA
  // DAC output is scaled by 8/16/128.
  // Then LPF input multiplies by 2X to get 4/8/64.        
  // SYN ranges should be roughly between 125fA/62.5fA/8fA and 125pA/62.5pA/8pA
  dac_info_[BDHornEP::DAC_SYN_EXC]     = {8  , (34 + 30) * 8};   // (34 + 30) * 8 = 512
  dac_info_[BDHornEP::DAC_SYN_DC]      = {16 , 544};             // 34 * 16 = 544
  dac_info_[BDHornEP::DAC_SYN_INH]     = {128, (34 - 30) * 128}; // (34 - 30) * 128 = 512
  dac_info_[BDHornEP::DAC_SYN_PU]      = {1  , 1024}; // roughly 1pA to 1nA
  dac_info_[BDHornEP::DAC_SYN_PD]      = {1  , 40}; // roughly 1pA to 1nA
  // DAC output is scaled by 160.
  // Then LPF leak multiplies by 8X to get 20.
  dac_info_[BDHornEP::DAC_SYN_LK]      = {160, 10}; // roughly 6.25fA to 6.25pA
  dac_info_[BDHornEP::DAC_DIFF_G]      = {1  , 1024}; // roughly 1pA to 1nA
  dac_info_[BDHornEP::DAC_DIFF_R]      = {1  , 1024}; // roughly 1pA to 1nA
  dac_info_[BDHornEP::DAC_SOMA_OFFSET] = {4  , 2}; // roughly 250fA to 250pA 
  dac_info_[BDHornEP::DAC_SOMA_REF]    = {1  , 10}; // roughly 1pA to 1nA

  //Init maps for AER address translation
  BDPars::InitAERToXY<12>(soma_aer_to_xy_, soma_xy_to_aer_);
  BDPars::InitAERToXY<10>(syn_aer_to_xy_, syn_xy_to_aer_);
  BDPars::InitAERToXY<8>(mem_aer_to_xy_, mem_xy_to_aer_);
}

// D is binary tree depth, not 4-ary tree depth, must be even
template <int D>
void BDPars::InitAERToXY(std::array<unsigned int, (1<<D)>& aer_to_xy, std::array<unsigned int, (1<<D)>& xy_to_aer) {
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
    aer_to_xy.at(aer_idx) = xy_idx;
    xy_to_aer.at(xy_idx) = aer_idx;
    // cout << "xy: " << xy_idx << "aer: " << aer_idx << endl;
  }
}

} // bdpars
} // bddriver
} // pystorm
