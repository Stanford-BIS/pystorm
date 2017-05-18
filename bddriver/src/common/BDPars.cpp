#include "BDPars.h"

#include <vector>
#include <string>
#include <unordered_map>

#include "common/binary_util.h"

namespace pystorm {
namespace bddriver {
namespace bdpars {

BDPars::BDPars() 
{
  //////////////////////////////////////////////////////
  // Funnel/Horn


  horn_.resize(LastHornLeafId+1);

  //                               {component type, component, route, route_len, data width, serialization, serialized data width, description}
  horn_[NeuronInject]            = {Input, static_cast<unsigned int>(InputSpikes)      , 14 , 4  , 11 , 1  , 11 , "direct spike injection to neuron array"};
  horn_[RI]                      = {Input, static_cast<unsigned int>(InputTags)        , 0  , 1  , 20 , 1  , 20 , "main tag input to FIFO"};
  horn_[PROG_AMMM]               = {Mem  , static_cast<unsigned int>(MM)               , 39 , 6  , 42 , 4  , 11 , "AM/MM programming/diagnostic port"}; // it's really both AM and MM
  horn_[PROG_PAT]                = {Mem  , static_cast<unsigned int>(PAT)              , 37 , 6  , 27 , 4  , 7  , "PAT programming/diagnostic port"};
  horn_[PROG_TAT0]               = {Mem  , static_cast<unsigned int>(TAT0)             , 65 , 7  , 31 , 4  , 8  , "TAT 0 programming/diagnostic port"};
  horn_[PROG_TAT1]               = {Mem  , static_cast<unsigned int>(TAT1)             , 67 , 7  , 31 , 4  , 8  , "TAT 1 programming/diagnostic port"};
  horn_[INIT_FIFO_DCT]           = {Input, static_cast<unsigned int>(DCTFIFOInputTags) , 70 , 7  , 11 , 1  , 11 , "inserts a tag into the DCT side of the FIFO with ct = 1, needed to clean initial FIFO state"};
  horn_[INIT_FIFO_HT]            = {Input, static_cast<unsigned int>(HTFIFOReset)      , 136, 8  , 1  , 1  , 1  , "trigger sets FIFO head/tail register to empty state"};
  horn_[TOGGLE_PRE_FIFO_leaf]    = {Reg  , static_cast<unsigned int>(TOGGLE_PRE_FIFO)  , 137, 8  , 2  , 1  , 2  , "toggles data/dump traffic for FIFO input"};
  horn_[TOGGLE_POST_FIFO0_leaf]  = {Reg  , static_cast<unsigned int>(TOGGLE_POST_FIFO0), 138, 8  , 2  , 1  , 2  , "toggles data/dump traffic for FIFO tag class 0 output"};
  horn_[TOGGLE_POST_FIFO1_leaf]  = {Reg  , static_cast<unsigned int>(TOGGLE_POST_FIFO1), 139, 8  , 2  , 1  , 2  , "toggles data/dump traffic for FIFO tag class 1 output"};
  horn_[NeuronDumpToggle_leaf]   = {Reg  , static_cast<unsigned int>(NeuronDumpToggle) , 15 , 4  , 2  , 1  , 2  , "toggles data/dump traffic for neuron array output"};
  horn_[NeuronConfig]            = {Input, static_cast<unsigned int>(TileSRAMInputs)   , 6  , 3  , 18 , 1  , 18 , "programming input for neuron array tile SRAM"};
  horn_[DAC0_leaf]               = {Reg  , static_cast<unsigned int>(DAC0)             , 176, 8  , 11 , 1  , 11 , "DIFF_G DAC bias value"};
  horn_[DAC1_leaf]               = {Reg  , static_cast<unsigned int>(DAC1)             , 177, 8  , 11 , 1  , 11 , "DIFF_R DAC bias value"};
  horn_[DAC2_leaf]               = {Reg  , static_cast<unsigned int>(DAC2)             , 178, 8  , 11 , 1  , 11 , "SOMA_OFFSET DAC bias value"};
  horn_[DAC3_leaf]               = {Reg  , static_cast<unsigned int>(DAC3)             , 179, 8  , 11 , 1  , 11 , "SYN_LK DAC bias value"};
  horn_[DAC4_leaf]               = {Reg  , static_cast<unsigned int>(DAC4)             , 180, 8  , 11 , 1  , 11 , "SYN_DC DAC bias value"};
  horn_[DAC5_leaf]               = {Reg  , static_cast<unsigned int>(DAC5)             , 181, 8  , 11 , 1  , 11 , "SYN_PD DAC bias value"};
  horn_[DAC6_leaf]               = {Reg  , static_cast<unsigned int>(DAC6)             , 182, 8  , 11 , 1  , 11 , "ADC_BIAS_2 DAC bias value"};
  horn_[DAC7_leaf]               = {Reg  , static_cast<unsigned int>(DAC7)             , 183, 8  , 11 , 1  , 11 , "ADC_BIAS_1 DAC bias value"};
  horn_[DAC8_leaf]               = {Reg  , static_cast<unsigned int>(DAC8)             , 184, 8  , 11 , 1  , 11 , "SOMA_REF DAC bias value"};
  horn_[DAC9_leaf]               = {Reg  , static_cast<unsigned int>(DAC9)             , 185, 8  , 11 , 1  , 11 , "SOMA_EXC DAC bias value"};
  horn_[DAC10_leaf]              = {Reg  , static_cast<unsigned int>(DAC10)            , 93 , 7  , 11 , 1  , 11 , "SOMA_INH DAC bias value"};
  horn_[DAC11_leaf]              = {Reg  , static_cast<unsigned int>(DAC11)            , 94 , 7  , 11 , 1  , 11 , "SYN_PU DAC bias value"};
  horn_[DAC12_leaf]              = {Reg  , static_cast<unsigned int>(DAC12)            , 95 , 7  , 11 , 1  , 11 , "UNUSED (ghost DAC)"};
  horn_[ADC_leaf]                = {Reg  , static_cast<unsigned int>(ADC)              , 10 , 4  , 3  , 1  , 3  , "ADC small/large current enable, output enable"};
  horn_[DELAY0_leaf]             = {Reg  , static_cast<unsigned int>(DELAY0)           , 64 , 7  , 8  , 1  , 8  , "FIFO:DCT delay line config"};
  horn_[DELAY1_leaf]             = {Reg  , static_cast<unsigned int>(DELAY1)           , 66 , 7  , 8  , 1  , 8  , "FIFO:PG delay line config"};
  horn_[DELAY2_leaf]             = {Reg  , static_cast<unsigned int>(DELAY2)           , 142, 8  , 8  , 1  , 8  , "TAT 0 delay line config"};
  horn_[DELAY3_leaf]             = {Reg  , static_cast<unsigned int>(DELAY3)           , 143, 8  , 8  , 1  , 8  , "TAT 1 delay line config"};
  horn_[DELAY4_leaf]             = {Reg  , static_cast<unsigned int>(DELAY4)           , 36 , 6  , 8  , 1  , 8  , "PAT delay line config"};
  horn_[DELAY5_leaf]             = {Reg  , static_cast<unsigned int>(DELAY5)           , 76 , 7  , 8  , 1  , 8  , "MM delay line config"};
  horn_[DELAY6_leaf]             = {Reg  , static_cast<unsigned int>(DELAY6)           , 77 , 7  , 8  , 1  , 8  , "AM delay line config"};


  //                               {component type, component, route, route_len, data width, serialization, serialized data width, description}
  funnel_.resize(LastFunnelLeafId+1);
  funnel_[RO_ACC]                = {Output, static_cast<unsigned int>(AccOutputTags), 1  , 2  , 28 , 1  , 28 , "tag output from accumulator"};
  funnel_[RO_TAT]                = {Output, static_cast<unsigned int>(TATOutputTags), 0  , 2  , 32 , 1  , 32 , "tag output from TAT"};
  funnel_[NRNI]                  = {Output, static_cast<unsigned int>(OutputSpikes) , 3  , 2  , 12 , 1  , 12 , "copy of traffic exiting neuron array"};
  funnel_[DUMP_AM]               = {Mem   , static_cast<unsigned int>(AM)           , 40 , 6  , 38 , 2  , 19 , "AM diagnostic read output"};
  funnel_[DUMP_MM]               = {Mem   , static_cast<unsigned int>(MM)           , 41 , 6  , 8  , 1  , 8  , "MM diagnostic read output"};
  funnel_[DUMP_PAT]              = {Mem   , static_cast<unsigned int>(PAT)          , 21 , 5  , 20 , 1  , 20 , "PAT diagnostic read output"};
  funnel_[DUMP_TAT0]             = {Mem   , static_cast<unsigned int>(TAT0)         , 8  , 4  , 29 , 1  , 29 , "TAT 0 diagnostic read output"};
  funnel_[DUMP_TAT1]             = {Mem   , static_cast<unsigned int>(TAT1)         , 9  , 4  , 29 , 1  , 29 , "TAT 1 diagnostic read output"};
  funnel_[DUMP_PRE_FIFO]         = {Output, static_cast<unsigned int>(PreFIFOTags)  , 45 , 6  , 20 , 1  , 20 , "copy of traffic entering FIFO"};
  funnel_[DUMP_POST_FIFO0]       = {Output, static_cast<unsigned int>(PostFIFOTags0), 46 , 6  , 19 , 1  , 19 , "copy of tag class 0 traffic exiting FIFO"};
  funnel_[DUMP_POST_FIFO1]       = {Output, static_cast<unsigned int>(PostFIFOTags1), 47 , 6  , 19 , 1  , 19 , "copy of tag class 1 traffic exiting FIFO"};
  funnel_[OVFLW0]                = {Output, static_cast<unsigned int>(OverflowTags0), 88 , 7  , 1  , 1  , 1  , "class 0 FIFO overflow warning"};
  funnel_[OVFLW1]                = {Output, static_cast<unsigned int>(OverflowTags1), 89 , 7  , 1  , 1  , 1  , "class 1 FIFO overflow warning"};

  //////////////////////////////////////////////////////
  // Registers
  
  WordStructure toggle_word = {{traffic_enable, 1}, {dump_enable, 1}};
  WordStructure DAC_word    = {{DAC_to_ADC_conn, 1}, {DAC_value, 10}};
  WordStructure ADC_word    = {{ADC_small_large_current_0, 1}, {ADC_small_large_current_1, 1}, {ADC_output_enable, 1}};
  WordStructure DELAY_word  = {{read_delay, 4}, {write_delay, 4}};

  reg_.resize(LastRegId+1);
  reg_[TOGGLE_PRE_FIFO]   = {toggle_word, TOGGLE_PRE_FIFO_leaf};
  reg_[TOGGLE_POST_FIFO0] = {toggle_word, TOGGLE_POST_FIFO0_leaf};
  reg_[TOGGLE_POST_FIFO1] = {toggle_word, TOGGLE_POST_FIFO1_leaf};
  reg_[NeuronDumpToggle]  = {toggle_word, NeuronDumpToggle_leaf};
  reg_[DAC0]              = {DAC_word,    DAC0_leaf};
  reg_[DAC1]              = {DAC_word,    DAC1_leaf};
  reg_[DAC2]              = {DAC_word,    DAC2_leaf};
  reg_[DAC3]              = {DAC_word,    DAC3_leaf};
  reg_[DAC4]              = {DAC_word,    DAC4_leaf};
  reg_[DAC5]              = {DAC_word,    DAC5_leaf};
  reg_[DAC6]              = {DAC_word,    DAC6_leaf};
  reg_[DAC7]              = {DAC_word,    DAC7_leaf};
  reg_[DAC8]              = {DAC_word,    DAC8_leaf};
  reg_[DAC9]              = {DAC_word,    DAC9_leaf};
  reg_[DAC10]             = {DAC_word,    DAC10_leaf};
  reg_[DAC11]             = {DAC_word,    DAC11_leaf};
  reg_[DAC12]             = {DAC_word,    DAC12_leaf};
  reg_[ADC]               = {ADC_word,    ADC_leaf};
  reg_[DELAY0]            = {DELAY_word,  DELAY0_leaf};
  reg_[DELAY1]            = {DELAY_word,  DELAY1_leaf};
  reg_[DELAY2]            = {DELAY_word,  DELAY2_leaf};
  reg_[DELAY3]            = {DELAY_word,  DELAY3_leaf};
  reg_[DELAY4]            = {DELAY_word,  DELAY4_leaf};
  reg_[DELAY5]            = {DELAY_word,  DELAY5_leaf};
  reg_[DELAY6]            = {DELAY_word,  DELAY6_leaf};

  //////////////////////////////////////////////////////
  // Memories
  
  // this is the packing of the stored value! goes in the data fields of programming words
  std::vector<WordStructure> AM_words  = {{{accumulator_value, 15}, 
                                           {threshold, 3}, 
                                           {stop, 1}, 
                                           {next_address, 19}}};
  std::vector<WordStructure> MM_words  = {{{weight, 8}}};
  std::vector<WordStructure> PAT_words = {{{AM_address, 10}, 
                                           {MM_address_lo, 8}, 
                                           {MM_address_hi, 2}}};
  std::vector<WordStructure> TAT_words = {{{AM_address, 10}, // acc type
                                           {MM_address, 16}},
                                          {{synapse_address_0, 11}, // spike type
                                           {synapse_sign_0, 1}, 
                                           {synapse_address_1, 11}, 
                                           {synapse_sign_1, 1}, 
                                           {unused, 2}},
                                          {{tag, 11}, // fanout type
                                           {global_route, 12}, 
                                           {unused,3}}};

  mem_.resize(LastMemId+1);
  mem_[AM]   = {1024,    AM_words,  PROG_AMMM, DUMP_AM};
  mem_[MM]   = {64*1024, MM_words,  PROG_AMMM, DUMP_MM};
  mem_[TAT0] = {1024,    TAT_words, PROG_TAT0, DUMP_TAT0};
  mem_[TAT1] = {1024,    TAT_words, PROG_TAT1, DUMP_TAT1};
  mem_[PAT]  = {64,      PAT_words, PROG_PAT,  DUMP_PAT};

  // these are the words you use to program the memory
  mem_prog_words_.resize(LastMemWordId+1);
  mem_prog_words_[PAT_write]             = {{address, 6}, {FIXED_0, 2}, {data, 20}};
  mem_prog_words_[PAT_read]              = {{address, 6}, {FIXED_1, 2}, {unused, 20}};

  mem_prog_words_[TAT_set_address]       = {{FIXED_0, 2}, {address, 10}, {unused, 19}};
  mem_prog_words_[TAT_write_increment]   = {{FIXED_1, 2}, {data, 29}};
  mem_prog_words_[TAT_read_increment]    = {{FIXED_2, 2}, {unused, 29}};

  mem_prog_words_[MM_set_address]        = {{FIXED_0, 2}, {address, 16}, {unused, 22}};
  mem_prog_words_[MM_write_increment]    = {{FIXED_1, 2}, {data, 8}, {unused, 30    }};
  mem_prog_words_[MM_read_increment]     = {{FIXED_2, 2}, {unused, 38}};
  mem_prog_words_[AM_set_address]        = {{FIXED_0, 2}, {address, 10}, {unused, 28}};
  mem_prog_words_[AM_read_write]         = {{FIXED_1, 2}, {data, 38}};
  mem_prog_words_[AM_increment]          = {{FIXED_2, 2}, {unused, 38}};

  mem_prog_words_[AM_encapsulation]      = {{FIXED_0, 1}, {payload, 40}, {stop, 1}};
  mem_prog_words_[MM_encapsulation]      = {{FIXED_1, 1}, {payload, 40}, {stop, 1}};


  //////////////////////////////////////////////////////
  // inputs
  
  input_.resize(LastInputId+1); 
  input_[InputTags]          = {{{count, 9}, {tag, 11}},                    RI};
  input_[DCTFIFOInputTags]   = {{{tag, 11}},                                INIT_FIFO_DCT};
  input_[InputSpikes]        = {{{synapse_sign, 1}, {synapse_address, 10}}, NeuronInject};


  //////////////////////////////////////////////////////
  // outputs
  
  output_.resize(LastOutputId+1); // XXX this is kind of janky
  output_[PreFIFOTags]   = {{{count, 9}, {tag, 11}},                     DUMP_PRE_FIFO};
  output_[PostFIFOTags0] = {{{count, 9}, {tag, 10}},                     DUMP_POST_FIFO0};
  output_[PostFIFOTags1] = {{{count, 9}, {tag, 10}},                     DUMP_POST_FIFO1};
  output_[OutputSpikes]  = {{{neuron_address, 12}},                      NRNI};
  output_[OverflowTags0] = {{{FIXED_1, 1}},                              OVFLW0};
  output_[OverflowTags1] = {{{FIXED_1, 1}},                              OVFLW1};
  output_[AccOutputTags] = {{{count, 9}, {tag, 11}, {global_route, 8}},  RO_ACC};
  output_[TATOutputTags] = {{{count, 9}, {tag, 11}, {global_route, 12}}, RO_TAT};

  //////////////////////////////////////////////////////
  // misc
  
  misc_widths_.resize(LastMiscWidthId+1);
  misc_widths_[BD_input] = 21;
  misc_widths_[BD_output] = 34;

  num_cores_ = 1;

  //////////////////////////////////////////////////////
  // Postprocessing

  // create direct-mapped tables used in encoding/decoding
  for (LeafInfo& leaf_info : horn_) {
    auto to_push = std::make_pair(leaf_info.route_val, leaf_info.route_len);
    horn_routes_.push_back(to_push);
  }

  for (LeafInfo& leaf_info : funnel_) {
    auto to_push = std::make_pair(leaf_info.route_val, leaf_info.route_len);
    funnel_routes_.push_back(to_push);
  }

}

RegId BDPars::DACSignalIdToDACRegisterId(DACSignalId id) const
{
  assert(false && "not implemented");
  return DAC0; // squelch compiler warning for now
}

} // bdpars
} // bddriver
} // pystorm
