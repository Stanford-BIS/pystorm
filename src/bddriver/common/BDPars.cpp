#include "BDPars.h"

#include <string>
#include <unordered_map>
#include <array>

namespace pystorm {
namespace bddriver {
namespace bdpars {

// clang-format off

BDPars::BDPars() {

  //////////////////////////////////////////////////////
  // FPGA downstream endpoints
  // BD inputs, registers, and channels
  
  //////////////////////////////////////////////////////
  // BD inputs

  // serialization is 1 for most leaves
  for (unsigned int i = 0; i < static_cast<unsigned int>(BDHornEP::COUNT); i++) {
    Dn_EP_size_[DnEPCodeFor(static_cast<BDHornEP>(i))]        = 24; // don't care: shorter than 24b
  }
  // is 2 for some leaves
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
    Up_EP_size_[UpEPCodeFor(static_cast<BDFunnelEP>(i))] = 24; // don't care, shorter than 24b
  }
  // serialization is 2 for a few others
  Up_EP_size_[UpEPCodeFor(BDFunnelEP::DUMP_AM)]          = 38;
  Up_EP_size_[UpEPCodeFor(BDFunnelEP::DUMP_TAT0)]        = 29;
  Up_EP_size_[UpEPCodeFor(BDFunnelEP::DUMP_TAT1)]        = 29;
  Up_EP_size_[UpEPCodeFor(BDFunnelEP::RO_ACC)]           = 28;
  Up_EP_size_[UpEPCodeFor(BDFunnelEP::RO_TAT)]           = 32;

  //////////////////////////////////////////////////////
  // FPGA outputs

  Up_EP_size_[UpEPCodeFor(FPGAOutputEP::UPSTREAM_HB)]    = 48;
  Up_EP_size_[UpEPCodeFor(FPGAOutputEP::SF_OUTPUT)]      = 48;
  Up_EP_size_[UpEPCodeFor(FPGAOutputEP::NOP)]            = 24;

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
  dac_info_[BDHornEP::DAC_ADC_BIAS_1]  = {25  , 1};
  dac_info_[BDHornEP::DAC_ADC_BIAS_1]  = {25  , 1};
  // DAC output is scaled by 8/16/128.
  // Then LPF input multiplies by 2X to get 4/8/64.
  dac_info_[BDHornEP::DAC_SYN_EXC]     = {8   , (34 + 30) * 8};
  dac_info_[BDHornEP::DAC_SYN_DC]      = {16  , 34 * 16};
  dac_info_[BDHornEP::DAC_SYN_INH]     = {128 , (34 - 30) * 128};
  dac_info_[BDHornEP::DAC_SYN_PU]      = {1   , 1024};
  dac_info_[BDHornEP::DAC_SYN_PD]      = {1   , 22};
  // DAC output is scaled by 160.
  // Then LPF leak multiplies by 8X to get 20.
  dac_info_[BDHornEP::DAC_SYN_LK]      = {160 , 10};
  dac_info_[BDHornEP::DAC_DIFF_G]      = {1   , 1024};
  dac_info_[BDHornEP::DAC_DIFF_R]      = {1   , 512};
  dac_info_[BDHornEP::DAC_SOMA_OFFSET] = {4   , 1};
  dac_info_[BDHornEP::DAC_SOMA_REF]    = {1   , 10};
}

} // bdpars
} // bddriver
} // pystorm
