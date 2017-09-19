#include "BDPars.h"

#include <string>
#include <unordered_map>
#include <array>

#include "common/binary_util.h"

namespace pystorm {
namespace bddriver {

// clang-format off

BDPars::BDPars() {

  //////////////////////////////////////////////////////
  // FPGA downstream endpoints
  // BD inputs, registers, and channels
  
  //////////////////////////////////////////////////////
  // BD inputs

  // serialization is 1 for most leaves
  for (unsigned int i = 0; i < static_cast<unsigned int>(BDHornEP::COUNT); i++) {
    BDHorn_serialization_[static_cast<BDHornEP>(i)] = 1;  
  }
  // serialization = 4 for a few others
  BDHorn_serialization_[BDHornEP::PROG_AMMM] = 4;  
  BDHorn_serialization_[BDHornEP::PROG_PAT]  = 4;  
  BDHorn_serialization_[BDHornEP::PROG_TAT0] = 4;
  BDHorn_serialization_[BDHornEP::PROG_TAT1] = 4;

  //////////////////////////////////////////////////////
  // FPGA Registers

  FPGA_reg_size_[FPGARegEP::SF_FILTS_USED]          = 1; 
  FPGA_reg_size_[FPGARegEP::SF_INCREMENT_CONSTANT]  = 2; 
  FPGA_reg_size_[FPGARegEP::SF_DECAY_CONSTANT]      = 2; 
  FPGA_reg_size_[FPGARegEP::SG_GENS_USED]           = 1; 
  FPGA_reg_size_[FPGARegEP::SG_GENS_EN]             = 16; 
  FPGA_reg_size_[FPGARegEP::TM_UNIT_LEN]            = 1; 
  FPGA_reg_size_[FPGARegEP::TM_PC_TIME_ELAPSED]     = 3; 
  FPGA_reg_size_[FPGARegEP::TM_PC_SEND_HB_UP_EVERY] = 3;
  FPGA_reg_size_[FPGARegEP::TM_PC_RESET_TIME]       = 1; 
  FPGA_reg_size_[FPGARegEP::TS_REPORT_TAGS]         = 1; 
  FPGA_reg_size_[FPGARegEP::BD_RESET]               = 1; 

  //////////////////////////////////////////////////////
  // FPGA Channels

  FPGA_channel_serialization_[FPGAChannelEP::SG_PROGRAM_MEM] = 4;

  //////////////////////////////////////////////////////
  // FPGA upstream endpoints
  // BD outputs, FPGA outputs
  
  // serialization is 1 for most leaves
  for (unsigned int i = 0; i < BDFunnelEP::COUNT; i++) {
    BDFunnel_serialization_[i] = 1;  
  }
  BDFunnel_serialization_[BDFunnelEP::DUMP_AM]   = 2;
  BDFunnel_serialization_[BDFunnelEP::DUMP_TAT0] = 2;
  BDFunnel_serialization_[BDFunnelEP::DUMP_TAT1] = 2;
  BDFunnel_serialization_[BDFunnelEP::RO_ACC]    = 2; 
  BDFunnel_serialization_[BDFunnelEP::RO_TAT]    = 2;

  //////////////////////////////////////////////////////
  // memory info

  mem_info_[BDMemId::AM]       = {1024,    BDHornEP::PROG_AMMM, BDFunnelEP::DUMP_AM,   BDHornEP::DELAY6};
  mem_info_[BDMemId::MM]       = {64*1024, BDHornEP::PROG_AMMM, BDFunnelEP::DUMP_MM,   BDHornEP::DELAY5};
  mem_info_[BDMemId::TAT0]     = {1024,    BDHornEP::PROG_TAT0, BDFunnelEP::DUMP_TAT0, BDHornEP::DELAY2};
  mem_info_[BDMemId::TAT1]     = {1024,    BDHornEP::PROG_TAT1, BDFunnelEP::DUMP_TAT1, BDHornEP::DELAY3};
  mem_info_[BDMemId::PAT]      = {64,      BDHornEP::PROG_PAT,  BDFunnelEP::DUMP_PAT,  BDHornEP::DELAY4};
  mem_info_[BDMemId::FIFO_DCT] = {2048,    BDHornEP::bad_hleaf, BDFunnelEP::bad_fleaf, BDHornEP::DELAY0};
  mem_info_[BDMemId::FIFO_PG]  = {2048,    BDHornEP::bad_hleaf, BDFunnelEP::bad_fleaf, BDHornEP::DELAY1};
}

uint8_t BDPars::GetDnEPCode(BDHornEP ep) {
  return static_cast<uint8_t>(ep);
}

uint8_t BDPars::GetDnEPCode(FPGARegEP ep) {
  return static_cast<uint8_t>(ep) + DnEPFPGARegOffset;
}

uint8_t BDPars::GetDnEPCode(FPGAChannelEP ep) {
  return static_cast<uint8_t>(ep) + DnEPFPGAChannelOffset;
}

}  // bddriver
}  // pystorm
