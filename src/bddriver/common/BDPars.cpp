#include "BDPars.h"

#include <string>
#include <unordered_map>
#include <array>

#include "common/binary_util.h"

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
    BDHorn_size_[static_cast<BDHornEP>(i)]          = 24; // don't care: shorter than 24b
    BDHorn_serialization_[static_cast<BDHornEP>(i)] = 1;
  }
  // serialization = 4 for a few longer words
  BDHorn_size_[BDHornEP::PROG_AMMM]          = 42;
  BDHorn_size_[BDHornEP::PROG_PAT]           = 27;
  BDHorn_size_[BDHornEP::PROG_TAT0]          = 31;
  BDHorn_size_[BDHornEP::PROG_TAT1]          = 31;
  BDHorn_serialization_[BDHornEP::PROG_AMMM] = 4;
  BDHorn_serialization_[BDHornEP::PROG_PAT]  = 4;
  BDHorn_serialization_[BDHornEP::PROG_TAT0] = 4;
  BDHorn_serialization_[BDHornEP::PROG_TAT1] = 4;

  //////////////////////////////////////////////////////
  // FPGA Registers

  FPGA_reg_size_[FPGARegEP::SF_FILTS_USED]          = 9; 
  FPGA_reg_size_[FPGARegEP::SF_INCREMENT_CONSTANT]  = 27; 
  FPGA_reg_size_[FPGARegEP::SF_DECAY_CONSTANT]      = 27; 
  FPGA_reg_size_[FPGARegEP::SG_GENS_USED]           = 8; 
  FPGA_reg_size_[FPGARegEP::SG_GENS_EN]             = 256; 
  FPGA_reg_size_[FPGARegEP::TM_UNIT_LEN]            = 16; 
  FPGA_reg_size_[FPGARegEP::TM_PC_TIME_ELAPSED]     = 48; 
  FPGA_reg_size_[FPGARegEP::TM_PC_SEND_HB_UP_EVERY] = 48;
  FPGA_reg_size_[FPGARegEP::TM_PC_RESET_TIME]       = 1; 
  FPGA_reg_size_[FPGARegEP::TS_REPORT_TAGS]         = 1; 
  FPGA_reg_size_[FPGARegEP::BD_RESET]               = 2; 

  //////////////////////////////////////////////////////
  // FPGA Channels

  FPGA_channel_size_[FPGAChannelEP::SG_PROGRAM_MEM] = 51;

  //////////////////////////////////////////////////////
  // FPGA upstream endpoints
  // BD outputs, FPGA outputs
  
  // serialization is 1 for most leaves
  for (unsigned int i = 0; i < static_cast<unsigned int>(BDFunnelEP::COUNT); i++) {
    BDFunnel_size_[static_cast<BDFunnelEP>(i)]          = 24; // don't care, shorter than 24b
    BDFunnel_serialization_[static_cast<BDFunnelEP>(i)] = 1;
  }
  // serialization is 2 for a few others
  BDFunnel_size_[BDFunnelEP::DUMP_AM]            = 38;
  BDFunnel_size_[BDFunnelEP::DUMP_TAT0]          = 29;
  BDFunnel_size_[BDFunnelEP::DUMP_TAT1]          = 29;
  BDFunnel_size_[BDFunnelEP::RO_ACC]             = 28;
  BDFunnel_size_[BDFunnelEP::RO_TAT]             = 32;
  BDFunnel_serialization_[BDFunnelEP::DUMP_AM]   = 2;
  BDFunnel_serialization_[BDFunnelEP::DUMP_TAT0] = 2;
  BDFunnel_serialization_[BDFunnelEP::DUMP_TAT1] = 2;
  BDFunnel_serialization_[BDFunnelEP::RO_ACC]    = 2;
  BDFunnel_serialization_[BDFunnelEP::RO_TAT]    = 2;

  //////////////////////////////////////////////////////
  // memory info

  mem_info_[BDMemId::AM]       = {1024,    BDHornEP::PROG_AMMM, BDFunnelEP::DUMP_AM,   BDHornEP::DELAY_AM};
  mem_info_[BDMemId::MM]       = {64*1024, BDHornEP::PROG_AMMM, BDFunnelEP::DUMP_MM,   BDHornEP::DELAY_MM};
  mem_info_[BDMemId::TAT0]     = {1024,    BDHornEP::PROG_TAT0, BDFunnelEP::DUMP_TAT0, BDHornEP::DELAY_TAT0};
  mem_info_[BDMemId::TAT1]     = {1024,    BDHornEP::PROG_TAT1, BDFunnelEP::DUMP_TAT1, BDHornEP::DELAY_TAT1};
  mem_info_[BDMemId::PAT]      = {64,      BDHornEP::PROG_PAT,  BDFunnelEP::DUMP_PAT,  BDHornEP::DELAY_PAT};
  mem_info_[BDMemId::FIFO_DCT] = {2048,    BDHornEP::COUNT,     BDFunnelEP::COUNT,     BDHornEP::DELAY_DCTFIFO};
  mem_info_[BDMemId::FIFO_PG]  = {2048,    BDHornEP::COUNT,     BDFunnelEP::COUNT,     BDHornEP::DELAY_PGFIFO};
}

} // bdpars
} // bddriver
} // pystorm
