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
  // Funnel/Horn


  //                               {component type, component, route, route_len, data width, serialization, serialized data width, description}
  bdendpoints_[NEURON_INJECT]           = {INPUT, static_cast<unsigned int>(INPUT_SPIKES)       , 7  , 4  , 11 , 1  , 11 , "direct spike injection to neuron array"};
  bdendpoints_[RI]                      = {INPUT, static_cast<unsigned int>(INPUT_TAGS)         , 0  , 1  , 20 , 1  , 20 , "main tag input to FIFO"};
  bdendpoints_[PROG_AMMM]               = {MEM , static_cast<unsigned int>(MM)                  , 57 , 6  , 42 , 4  , 11 , "AM/MM programming/diagnostic port"}; // it's really both AM and MM
  bdendpoints_[PROG_PAT]                = {MEM , static_cast<unsigned int>(PAT)                 , 41 , 6  , 27 , 4  , 7  , "PAT programming/diagnostic port"};
  bdendpoints_[PROG_TAT0]               = {MEM , static_cast<unsigned int>(TAT0)                , 65 , 7  , 31 , 4  , 8  , "TAT 0 programming/diagnostic port"};
  bdendpoints_[PROG_TAT1]               = {MEM , static_cast<unsigned int>(TAT1)                , 97 , 7  , 31 , 4  , 8  , "TAT 1 programming/diagnostic port"};
  bdendpoints_[INIT_FIFO_DCT]           = {INPUT, static_cast<unsigned int>(DCT_FIFO_INPUT_TAGS), 49 , 7  , 11 , 1  , 11 , "inserts a tag into the DCT side of the FIFO with ct = 1, needed to clean initial FIFO state"};
  bdendpoints_[INIT_FIFO_HT]            = {INPUT, static_cast<unsigned int>(HT_FIFO_RESET)      , 17 , 8  , 1  , 1  , 1  , "trigger sets FIFO head/tail register to empty state"};
  bdendpoints_[TOGGLE_PRE_FIFO_LEAF]    = {REG  , static_cast<unsigned int>(TOGGLE_PRE_FIFO)    , 145, 8  , 2  , 1  , 2  , "toggles data/dump traffic for FIFO input"};
  bdendpoints_[TOGGLE_POST_FIFO0_LEAF]  = {REG  , static_cast<unsigned int>(TOGGLE_POST_FIFO0)  , 81 , 8  , 2  , 1  , 2  , "toggles data/dump traffic for FIFO tag class 0 output"};
  bdendpoints_[TOGGLE_POST_FIFO1_LEAF]  = {REG  , static_cast<unsigned int>(TOGGLE_POST_FIFO1)  , 209, 8  , 2  , 1  , 2  , "toggles data/dump traffic for FIFO tag class 1 output"};
  bdendpoints_[NEURON_DUMP_TOGGLE_LEAF] = {REG  , static_cast<unsigned int>(NEURON_DUMP_TOGGLE) , 15 , 4  , 2  , 1  , 2  , "toggles data/dump traffic for neuron array output"};
  bdendpoints_[NEURON_CONFIG]           = {INPUT, static_cast<unsigned int>(TILE_SRAM_INPUTS)   , 3  , 3  , 18 , 1  , 18 , "programming input for neuron array tile SRAM"};
  bdendpoints_[DAC0_LEAF]               = {REG  , static_cast<unsigned int>(DAC0)               , 13 , 8  , 11 , 1  , 11 , "DIFF_G DAC bias value"};
  bdendpoints_[DAC1_LEAF]               = {REG  , static_cast<unsigned int>(DAC1)               , 141, 8  , 11 , 1  , 11 , "DIFF_R DAC bias value"};
  bdendpoints_[DAC2_LEAF]               = {REG  , static_cast<unsigned int>(DAC2)               , 77 , 8  , 11 , 1  , 11 , "SOMA_OFFSET DAC bias value"};
  bdendpoints_[DAC3_LEAF]               = {REG  , static_cast<unsigned int>(DAC3)               , 205, 8  , 11 , 1  , 11 , "SYN_LK DAC bias value"};
  bdendpoints_[DAC4_LEAF]               = {REG  , static_cast<unsigned int>(DAC4)               , 45 , 8  , 11 , 1  , 11 , "SYN_DC DAC bias value"};
  bdendpoints_[DAC5_LEAF]               = {REG  , static_cast<unsigned int>(DAC5)               , 173, 8  , 11 , 1  , 11 , "SYN_PD DAC bias value"};
  bdendpoints_[DAC6_LEAF]               = {REG  , static_cast<unsigned int>(DAC6)               , 109, 8  , 11 , 1  , 11 , "ADC_BIAS_2 DAC bias value"};
  bdendpoints_[DAC7_LEAF]               = {REG  , static_cast<unsigned int>(DAC7)               , 237, 8  , 11 , 1  , 11 , "ADC_BIAS_1 DAC bias value"};
  bdendpoints_[DAC8_LEAF]               = {REG  , static_cast<unsigned int>(DAC8)               , 29 , 8  , 11 , 1  , 11 , "SOMA_REF DAC bias value"};
  bdendpoints_[DAC9_LEAF]               = {REG  , static_cast<unsigned int>(DAC9)               , 157, 8  , 11 , 1  , 11 , "SOMA_EXC DAC bias value"};
  bdendpoints_[DAC10_LEAF]              = {REG  , static_cast<unsigned int>(DAC10)              , 93 , 7  , 11 , 1  , 11 , "SOMA_INH DAC bias value"};
  bdendpoints_[DAC11_LEAF]              = {REG  , static_cast<unsigned int>(DAC11)              , 61 , 7  , 11 , 1  , 11 , "SYN_PU DAC bias value"};
  bdendpoints_[DAC12_LEAF]              = {REG  , static_cast<unsigned int>(DAC12)              , 125, 7  , 11 , 1  , 11 , "UNUSED (ghost DAC)"};
  bdendpoints_[ADC_LEAF]                = {REG  , static_cast<unsigned int>(ADC)                , 5  , 4  , 3  , 1  , 3  , "ADC small/large current enable, output enable"};
  bdendpoints_[DELAY0_LEAF]             = {REG  , static_cast<unsigned int>(DELAY0)             , 1  , 7  , 8  , 1  , 8  , "FIFO:DCT delay line config"};
  bdendpoints_[DELAY1_LEAF]             = {REG  , static_cast<unsigned int>(DELAY1)             , 33 , 7  , 8  , 1  , 8  , "FIFO:PG delay line config"};
  bdendpoints_[DELAY2_LEAF]             = {REG  , static_cast<unsigned int>(DELAY2)             , 113, 8  , 8  , 1  , 8  , "TAT 0 delay line config"};
  bdendpoints_[DELAY3_LEAF]             = {REG  , static_cast<unsigned int>(DELAY3)             , 241, 8  , 8  , 1  , 8  , "TAT 1 delay line config"};
  bdendpoints_[DELAY4_LEAF]             = {REG  , static_cast<unsigned int>(DELAY4)             , 9  , 6  , 8  , 1  , 8  , "PAT delay line config"};
  bdendpoints_[DELAY5_LEAF]             = {REG  , static_cast<unsigned int>(DELAY5)             , 25 , 7  , 8  , 1  , 8  , "MM delay line config"};
  bdendpoints_[DELAY6_LEAF]             = {REG  , static_cast<unsigned int>(DELAY6)             , 89 , 7  , 8  , 1  , 8  , "AM delay line config"};
  for(unsigned int idx = 34; idx < 128; ++idx){
    bdendpoints_[idx] = {FPGA_REG, 256 + idx, 0, 2, 24, 1, 24, "FPGA dummy endpoints"};
  }
  for(unsigned int idx = static_cast<unsigned int>(REG_OFFSET); idx < static_cast<unsigned int>(CHANNEL_OFFSET); ++idx){
    bdendpoints_[idx] = {FPGA_REG, idx - static_cast<unsigned int>(REG_OFFSET), 128, 2, 24, 1, 24, "FPGA register configuration"};
  }
  for(unsigned int idx = static_cast<unsigned int>(CHANNEL_OFFSET); idx < static_cast<unsigned int>(BDEndPointIdCount); ++idx){
    bdendpoints_[idx] = {FPGA_CHANNEL, idx - static_cast<unsigned int>(CHANNEL_OFFSET), 192, 2, 24, 1, 24, "FPGA channel configuration"};
  }

  //                               {component type, component, route, route_len, data width, serialization, serialized data width, description}
  bdstartpoints_[RO_ACC]                = {OUTPUT, static_cast<unsigned int>(ACC_OUTPUT_TAGS), 1  , 2  , 28 , 1  , 28 , "tag output from accumulator"};
  bdstartpoints_[RO_TAT]                = {OUTPUT, static_cast<unsigned int>(TAT_OUTPUT_TAGS), 0  , 2  , 32 , 1  , 32 , "tag output from TAT"};
  bdstartpoints_[NRNI]                  = {OUTPUT, static_cast<unsigned int>(OUTPUT_SPIKES)  , 3  , 2  , 12 , 1  , 12 , "copy of traffic exiting neuron array"};
  bdstartpoints_[DUMP_AM]               = {MEM  , static_cast<unsigned int>(AM)              , 40 , 6  , 38 , 2  , 19 , "AM diagnostic read output"};
  bdstartpoints_[DUMP_MM]               = {MEM  , static_cast<unsigned int>(MM)              , 41 , 6  , 8  , 1  , 8  , "MM diagnostic read output"};
  bdstartpoints_[DUMP_PAT]              = {MEM  , static_cast<unsigned int>(PAT)             , 21 , 5  , 20 , 1  , 20 , "PAT diagnostic read output"};
  bdstartpoints_[DUMP_TAT0]             = {MEM  , static_cast<unsigned int>(TAT0)            , 8  , 4  , 29 , 1  , 29 , "TAT 0 diagnostic read output"};
  bdstartpoints_[DUMP_TAT1]             = {MEM  , static_cast<unsigned int>(TAT1)            , 9  , 4  , 29 , 1  , 29 , "TAT 1 diagnostic read output"};
  bdstartpoints_[DUMP_PRE_FIFO]         = {OUTPUT, static_cast<unsigned int>(PRE_FIFO_TAGS)  , 45 , 6  , 20 , 1  , 20 , "copy of traffic entering FIFO"};
  bdstartpoints_[DUMP_POST_FIFO0]       = {OUTPUT, static_cast<unsigned int>(POST_FIFO_TAGS0), 46 , 6  , 19 , 1  , 19 , "copy of tag class 0 traffic exiting FIFO"};
  bdstartpoints_[DUMP_POST_FIFO1]       = {OUTPUT, static_cast<unsigned int>(POST_FIFO_TAGS1), 47 , 6  , 19 , 1  , 19 , "copy of tag class 1 traffic exiting FIFO"};
  bdstartpoints_[OVFLW0]                = {OUTPUT, static_cast<unsigned int>(OVERFLOW_TAGS0) , 88 , 7  , 1  , 1  , 1  , "class 0 FIFO overflow warning"};
  bdstartpoints_[OVFLW1]                = {OUTPUT, static_cast<unsigned int>(OVERFLOW_TAGS1) , 89 , 7  , 1  , 1  , 1  , "class 1 FIFO overflow warning"};

  //////////////////////////////////////////////////////
  // Registers

  reg_[TOGGLE_PRE_FIFO]    = {TOGGLE_PRE_FIFO_LEAF};
  reg_[TOGGLE_POST_FIFO0]  = {TOGGLE_POST_FIFO0_LEAF};
  reg_[TOGGLE_POST_FIFO1]  = {TOGGLE_POST_FIFO1_LEAF};
  reg_[NEURON_DUMP_TOGGLE] = {NEURON_DUMP_TOGGLE_LEAF};
  reg_[DAC0]               = {DAC0_LEAF};
  reg_[DAC1]               = {DAC1_LEAF};
  reg_[DAC2]               = {DAC2_LEAF};
  reg_[DAC3]               = {DAC3_LEAF};
  reg_[DAC4]               = {DAC4_LEAF};
  reg_[DAC5]               = {DAC5_LEAF};
  reg_[DAC6]               = {DAC6_LEAF};
  reg_[DAC7]               = {DAC7_LEAF};
  reg_[DAC8]               = {DAC8_LEAF};
  reg_[DAC9]               = {DAC9_LEAF};
  reg_[DAC10]              = {DAC10_LEAF};
  reg_[DAC11]              = {DAC11_LEAF};
  reg_[DAC12]              = {DAC12_LEAF};
  reg_[ADC]                = {ADC_LEAF};
  reg_[DELAY0]             = {DELAY0_LEAF};
  reg_[DELAY1]             = {DELAY1_LEAF};
  reg_[DELAY2]             = {DELAY2_LEAF};
  reg_[DELAY3]             = {DELAY3_LEAF};
  reg_[DELAY4]             = {DELAY4_LEAF};
  reg_[DELAY5]             = {DELAY5_LEAF};
  reg_[DELAY6]             = {DELAY6_LEAF};

  //////////////////////////////////////////////////////
  // Memories

  // for the FIFO, which doesn't have prog/dump ports like the others, have to put a dummy value in
  BDEndPointId   bad_hleaf = static_cast<BDEndPointId>(BDEndPointIdCount);
  BDStartPointId bad_fleaf = static_cast<BDStartPointId>(BDStartPointIdCount);

  mem_[AM]       = {1024,    PROG_AMMM, DUMP_AM,   DELAY6};
  mem_[MM]       = {64*1024, PROG_AMMM, DUMP_MM,   DELAY5};
  mem_[TAT0]     = {1024,    PROG_TAT0, DUMP_TAT0, DELAY2};
  mem_[TAT1]     = {1024,    PROG_TAT1, DUMP_TAT1, DELAY3};
  mem_[PAT]      = {64,      PROG_PAT,  DUMP_PAT,  DELAY4};
  mem_[FIFO_DCT] = {2048,    bad_hleaf, bad_fleaf, DELAY0};
  mem_[FIFO_PG]  = {2048,    bad_hleaf, bad_fleaf, DELAY1};

  //////////////////////////////////////////////////////
  // inputs

  input_[INPUT_TAGS]          = {RI};
  input_[DCT_FIFO_INPUT_TAGS] = {INIT_FIFO_DCT};
  input_[INPUT_SPIKES]        = {NEURON_INJECT};


  //////////////////////////////////////////////////////
  // outputs

  output_[PRE_FIFO_TAGS]   = {DUMP_PRE_FIFO};
  output_[POST_FIFO_TAGS0] = {DUMP_POST_FIFO0};
  output_[POST_FIFO_TAGS1] = {DUMP_POST_FIFO1};
  output_[OUTPUT_SPIKES]   = {NRNI};
  output_[OVERFLOW_TAGS0]  = {OVFLW0};
  output_[OVERFLOW_TAGS1]  = {OVFLW1};
  output_[ACC_OUTPUT_TAGS] = {RO_ACC};
  output_[TAT_OUTPUT_TAGS] = {RO_TAT};

  //////////////////////////////////////////////////////
  // misc

  misc_widths_[BD_INPUT] = 21;
  misc_widths_[BD_OUTPUT] = 34;

  num_cores_ = 1;

  //////////////////////////////////////////////////////
  // Postprocessing

}

RegId BDPars::DACSignalIdToDACRegisterId(DACSignalId id) const {
  assert(false && "not implemented");
  return DAC0;  // squelch compiler warning for now
}

}  // bdpars
}  // bddriver
}  // pystorm
