#include "BDPars.h"

#include <string>
#include <unordered_map>
#include <vector>

#include "common/binary_util.h"

namespace pystorm {
namespace bddriver {
namespace bdpars {

// clang-format off

BDPars::BDPars() {
  //////////////////////////////////////////////////////
  // Funnel/Horn


  horn_.resize(LastHornLeafId+1);

  //                               {component type, component, route, route_len, data width, serialization, serialized data width, description}
  horn_[NEURON_INJECT]           = {INPUT, static_cast<unsigned int>(INPUT_SPIKES)       , 7  , 4  , 11 , 1  , 11 , "direct spike injection to neuron array"};
  horn_[RI]                      = {INPUT, static_cast<unsigned int>(INPUT_TAGS)         , 0  , 1  , 20 , 1  , 20 , "main tag input to FIFO"};
  horn_[PROG_AMMM]               = {MEM , static_cast<unsigned int>(MM)                  , 57 , 6  , 42 , 4  , 11 , "AM/MM programming/diagnostic port"}; // it's really both AM and MM
  horn_[PROG_PAT]                = {MEM , static_cast<unsigned int>(PAT)                 , 41 , 6  , 27 , 4  , 7  , "PAT programming/diagnostic port"};
  horn_[PROG_TAT0]               = {MEM , static_cast<unsigned int>(TAT0)                , 65 , 7  , 31 , 4  , 8  , "TAT 0 programming/diagnostic port"};
  horn_[PROG_TAT1]               = {MEM , static_cast<unsigned int>(TAT1)                , 97 , 7  , 31 , 4  , 8  , "TAT 1 programming/diagnostic port"};
  horn_[INIT_FIFO_DCT]           = {INPUT, static_cast<unsigned int>(DCT_FIFO_INPUT_TAGS), 49 , 7  , 11 , 1  , 11 , "inserts a tag into the DCT side of the FIFO with ct = 1, needed to clean initial FIFO state"};
  horn_[INIT_FIFO_HT]            = {INPUT, static_cast<unsigned int>(HT_FIFO_RESET)      , 17 , 8  , 1  , 1  , 1  , "trigger sets FIFO head/tail register to empty state"};
  horn_[TOGGLE_PRE_FIFO_LEAF]    = {REG  , static_cast<unsigned int>(TOGGLE_PRE_FIFO)    , 145, 8  , 2  , 1  , 2  , "toggles data/dump traffic for FIFO input"};
  horn_[TOGGLE_POST_FIFO0_LEAF]  = {REG  , static_cast<unsigned int>(TOGGLE_POST_FIFO0)  , 81 , 8  , 2  , 1  , 2  , "toggles data/dump traffic for FIFO tag class 0 output"};
  horn_[TOGGLE_POST_FIFO1_LEAF]  = {REG  , static_cast<unsigned int>(TOGGLE_POST_FIFO1)  , 209, 8  , 2  , 1  , 2  , "toggles data/dump traffic for FIFO tag class 1 output"};
  horn_[NEURON_DUMP_TOGGLE_LEAF] = {REG  , static_cast<unsigned int>(NEURON_DUMP_TOGGLE) , 15 , 4  , 2  , 1  , 2  , "toggles data/dump traffic for neuron array output"};
  horn_[NEURON_CONFIG]           = {INPUT, static_cast<unsigned int>(TILE_SRAM_INPUTS)   , 3  , 3  , 18 , 1  , 18 , "programming input for neuron array tile SRAM"};
  horn_[DAC0_LEAF]               = {REG  , static_cast<unsigned int>(DAC0)               , 13 , 8  , 11 , 1  , 11 , "DIFF_G DAC bias value"};
  horn_[DAC1_LEAF]               = {REG  , static_cast<unsigned int>(DAC1)               , 141, 8  , 11 , 1  , 11 , "DIFF_R DAC bias value"};
  horn_[DAC2_LEAF]               = {REG  , static_cast<unsigned int>(DAC2)               , 77 , 8  , 11 , 1  , 11 , "SOMA_OFFSET DAC bias value"};
  horn_[DAC3_LEAF]               = {REG  , static_cast<unsigned int>(DAC3)               , 205, 8  , 11 , 1  , 11 , "SYN_LK DAC bias value"};
  horn_[DAC4_LEAF]               = {REG  , static_cast<unsigned int>(DAC4)               , 45 , 8  , 11 , 1  , 11 , "SYN_DC DAC bias value"};
  horn_[DAC5_LEAF]               = {REG  , static_cast<unsigned int>(DAC5)               , 173, 8  , 11 , 1  , 11 , "SYN_PD DAC bias value"};
  horn_[DAC6_LEAF]               = {REG  , static_cast<unsigned int>(DAC6)               , 109, 8  , 11 , 1  , 11 , "ADC_BIAS_2 DAC bias value"};
  horn_[DAC7_LEAF]               = {REG  , static_cast<unsigned int>(DAC7)               , 237, 8  , 11 , 1  , 11 , "ADC_BIAS_1 DAC bias value"};
  horn_[DAC8_LEAF]               = {REG  , static_cast<unsigned int>(DAC8)               , 29 , 8  , 11 , 1  , 11 , "SOMA_REF DAC bias value"};
  horn_[DAC9_LEAF]               = {REG  , static_cast<unsigned int>(DAC9)               , 157, 8  , 11 , 1  , 11 , "SOMA_EXC DAC bias value"};
  horn_[DAC10_LEAF]              = {REG  , static_cast<unsigned int>(DAC10)              , 93 , 7  , 11 , 1  , 11 , "SOMA_INH DAC bias value"};
  horn_[DAC11_LEAF]              = {REG  , static_cast<unsigned int>(DAC11)              , 61 , 7  , 11 , 1  , 11 , "SYN_PU DAC bias value"};
  horn_[DAC12_LEAF]              = {REG  , static_cast<unsigned int>(DAC12)              , 125, 7  , 11 , 1  , 11 , "UNUSED (ghost DAC)"};
  horn_[ADC_LEAF]                = {REG  , static_cast<unsigned int>(ADC)                , 5  , 4  , 3  , 1  , 3  , "ADC small/large current enable, output enable"};
  horn_[DELAY0_LEAF]             = {REG  , static_cast<unsigned int>(DELAY0)             , 1  , 7  , 8  , 1  , 8  , "FIFO:DCT delay line config"};
  horn_[DELAY1_LEAF]             = {REG  , static_cast<unsigned int>(DELAY1)             , 33 , 7  , 8  , 1  , 8  , "FIFO:PG delay line config"};
  horn_[DELAY2_LEAF]             = {REG  , static_cast<unsigned int>(DELAY2)             , 113, 8  , 8  , 1  , 8  , "TAT 0 delay line config"};
  horn_[DELAY3_LEAF]             = {REG  , static_cast<unsigned int>(DELAY3)             , 241, 8  , 8  , 1  , 8  , "TAT 1 delay line config"};
  horn_[DELAY4_LEAF]             = {REG  , static_cast<unsigned int>(DELAY4)             , 9  , 6  , 8  , 1  , 8  , "PAT delay line config"};
  horn_[DELAY5_LEAF]             = {REG  , static_cast<unsigned int>(DELAY5)             , 25 , 7  , 8  , 1  , 8  , "MM delay line config"};
  horn_[DELAY6_LEAF]             = {REG  , static_cast<unsigned int>(DELAY6)             , 89 , 7  , 8  , 1  , 8  , "AM delay line config"};

  //                               {component type, component, route, route_len, data width, serialization, serialized data width, description}
  funnel_.resize(LastFunnelLeafId+1);
  funnel_[RO_ACC]                = {OUTPUT, static_cast<unsigned int>(ACC_OUTPUT_TAGS), 1  , 2  , 28 , 1  , 28 , "tag output from accumulator"};
  funnel_[RO_TAT]                = {OUTPUT, static_cast<unsigned int>(TAT_OUTPUT_TAGS), 0  , 2  , 32 , 1  , 32 , "tag output from TAT"};
  funnel_[NRNI]                  = {OUTPUT, static_cast<unsigned int>(OUTPUT_SPIKES)  , 3  , 2  , 12 , 1  , 12 , "copy of traffic exiting neuron array"};
  funnel_[DUMP_AM]               = {MEM  , static_cast<unsigned int>(AM)              , 40 , 6  , 38 , 2  , 19 , "AM diagnostic read output"};
  funnel_[DUMP_MM]               = {MEM  , static_cast<unsigned int>(MM)              , 41 , 6  , 8  , 1  , 8  , "MM diagnostic read output"};
  funnel_[DUMP_PAT]              = {MEM  , static_cast<unsigned int>(PAT)             , 21 , 5  , 20 , 1  , 20 , "PAT diagnostic read output"};
  funnel_[DUMP_TAT0]             = {MEM  , static_cast<unsigned int>(TAT0)            , 8  , 4  , 29 , 1  , 29 , "TAT 0 diagnostic read output"};
  funnel_[DUMP_TAT1]             = {MEM  , static_cast<unsigned int>(TAT1)            , 9  , 4  , 29 , 1  , 29 , "TAT 1 diagnostic read output"};
  funnel_[DUMP_PRE_FIFO]         = {OUTPUT, static_cast<unsigned int>(PRE_FIFO_TAGS)  , 45 , 6  , 20 , 1  , 20 , "copy of traffic entering FIFO"};
  funnel_[DUMP_POST_FIFO0]       = {OUTPUT, static_cast<unsigned int>(POST_FIFO_TAGS0), 46 , 6  , 19 , 1  , 19 , "copy of tag class 0 traffic exiting FIFO"};
  funnel_[DUMP_POST_FIFO1]       = {OUTPUT, static_cast<unsigned int>(POST_FIFO_TAGS1), 47 , 6  , 19 , 1  , 19 , "copy of tag class 1 traffic exiting FIFO"};
  funnel_[OVFLW0]                = {OUTPUT, static_cast<unsigned int>(OVERFLOW_TAGS0) , 88 , 7  , 1  , 1  , 1  , "class 0 FIFO overflow warning"};
  funnel_[OVFLW1]                = {OUTPUT, static_cast<unsigned int>(OVERFLOW_TAGS1) , 89 , 7  , 1  , 1  , 1  , "class 1 FIFO overflow warning"};

  //////////////////////////////////////////////////////
  // Registers
  
  WordStructure TOGGLE_word = {{TRAFFIC_ENABLE, 1}, {DUMP_ENABLE, 1}};
  WordStructure DAC_word    = {{DAC_TO_ADC_CONN, 1}, {DAC_VALUE, 10}};
  WordStructure ADC_word    = {{ADC_SMALL_LARGE_CURRENT_0, 1}, {ADC_SMALL_LARGE_CURRENT_1, 1}, {ADC_OUTPUT_ENABLE, 1}};
  WordStructure DELAY_word  = {{READ_DELAY, 4}, {WRITE_DELAY, 4}};

  reg_.resize(LastRegId+1);
  reg_[TOGGLE_PRE_FIFO]    = {TOGGLE_word, TOGGLE_PRE_FIFO_LEAF};
  reg_[TOGGLE_POST_FIFO0]  = {TOGGLE_word, TOGGLE_POST_FIFO0_LEAF};
  reg_[TOGGLE_POST_FIFO1]  = {TOGGLE_word, TOGGLE_POST_FIFO1_LEAF};
  reg_[NEURON_DUMP_TOGGLE] = {TOGGLE_word, NEURON_DUMP_TOGGLE_LEAF};
  reg_[DAC0]               = {DAC_word,    DAC0_LEAF};
  reg_[DAC1]               = {DAC_word,    DAC1_LEAF};
  reg_[DAC2]               = {DAC_word,    DAC2_LEAF};
  reg_[DAC3]               = {DAC_word,    DAC3_LEAF};
  reg_[DAC4]               = {DAC_word,    DAC4_LEAF};
  reg_[DAC5]               = {DAC_word,    DAC5_LEAF};
  reg_[DAC6]               = {DAC_word,    DAC6_LEAF};
  reg_[DAC7]               = {DAC_word,    DAC7_LEAF};
  reg_[DAC8]               = {DAC_word,    DAC8_LEAF};
  reg_[DAC9]               = {DAC_word,    DAC9_LEAF};
  reg_[DAC10]              = {DAC_word,    DAC10_LEAF};
  reg_[DAC11]              = {DAC_word,    DAC11_LEAF};
  reg_[DAC12]              = {DAC_word,    DAC12_LEAF};
  reg_[ADC]                = {ADC_word,    ADC_LEAF};
  reg_[DELAY0]             = {DELAY_word,  DELAY0_LEAF};
  reg_[DELAY1]             = {DELAY_word,  DELAY1_LEAF};
  reg_[DELAY2]             = {DELAY_word,  DELAY2_LEAF};
  reg_[DELAY3]             = {DELAY_word,  DELAY3_LEAF};
  reg_[DELAY4]             = {DELAY_word,  DELAY4_LEAF};
  reg_[DELAY5]             = {DELAY_word,  DELAY5_LEAF};
  reg_[DELAY6]             = {DELAY_word,  DELAY6_LEAF};

  //////////////////////////////////////////////////////
  // Memories
  
  // this is the packing of the stored value! goes in the data fields of programming words
  std::vector<WordStructure> AM_words  = 
      {{{ACCUMULATOR_VALUE, 15}, {THRESHOLD, 3}, {STOP, 1}, {NEXT_ADDRESS, 19}}};
  std::vector<WordStructure> MM_words  = 
      {{{WEIGHT, 8}}};
  std::vector<WordStructure> PAT_words = 
      {{{AM_ADDRESS, 10}, {MM_ADDRESS_LO, 8}, {MM_ADDRESS_HI, 2}}};
  std::vector<WordStructure> TAT_words = 
      {
          {{STOP, 1}, {FIXED_0, 2}, {AM_ADDRESS, 10}, {MM_ADDRESS, 16}}, // ACC TYPE 
          {{STOP, 1}, {FIXED_1, 2}, {SYNAPSE_ADDRESS_0, 11}, {SYNAPSE_SIGN_0, 1}, {SYNAPSE_ADDRESS_1, 11}, {SYNAPSE_SIGN_1, 1}, {UNUSED, 2}}, // SPIKE TYPE
          {{STOP, 1}, {FIXED_2, 2}, {TAG, 11}, {GLOBAL_ROUTE, 12}, {UNUSED,3}} // FANOUT TYPE
      };

  mem_.resize(LastMemId+1);
  mem_[AM]   = {1024,    AM_words,  PROG_AMMM, DUMP_AM};
  mem_[MM]   = {64*1024, MM_words,  PROG_AMMM, DUMP_MM};
  mem_[TAT0] = {1024,    TAT_words, PROG_TAT0, DUMP_TAT0};
  mem_[TAT1] = {1024,    TAT_words, PROG_TAT1, DUMP_TAT1};
  mem_[PAT]  = {64,      PAT_words, PROG_PAT,  DUMP_PAT};

  // these are the words you use to program the memory
  mem_prog_words_.resize(LastMemWordId+1);
  mem_prog_words_[PAT_WRITE]             = {{ADDRESS, 6}, {FIXED_0, 1}, {DATA, 20}};
  mem_prog_words_[PAT_READ]              = {{ADDRESS, 6}, {FIXED_1, 1}, {UNUSED, 20}};

  mem_prog_words_[TAT_SET_ADDRESS]       = {{FIXED_0, 2}, {ADDRESS, 10}, {UNUSED, 19}};
  mem_prog_words_[TAT_WRITE_INCREMENT]   = {{FIXED_1, 2}, {DATA, 29}};
  mem_prog_words_[TAT_READ_INCREMENT]    = {{FIXED_2, 2}, {UNUSED, 29}};

  mem_prog_words_[MM_SET_ADDRESS]        = {{FIXED_0, 2}, {ADDRESS, 16}, {UNUSED, 22}};
  mem_prog_words_[MM_WRITE_INCREMENT]    = {{FIXED_1, 2}, {DATA, 8}, {UNUSED, 30    }};
  mem_prog_words_[MM_READ_INCREMENT]     = {{FIXED_2, 2}, {UNUSED, 38}};
  mem_prog_words_[AM_SET_ADDRESS]        = {{FIXED_0, 2}, {ADDRESS, 10}, {UNUSED, 28}};
  mem_prog_words_[AM_READ_WRITE]         = {{FIXED_1, 2}, {DATA, 38}};
  mem_prog_words_[AM_INCREMENT]          = {{FIXED_2, 2}, {UNUSED, 38}};

  mem_prog_words_[AM_ENCAPSULATION]      = {{FIXED_0, 1}, {PAYLOAD, 40}, {AMMM_STOP, 1}};
  mem_prog_words_[MM_ENCAPSULATION]      = {{FIXED_1, 1}, {PAYLOAD, 40}, {AMMM_STOP, 1}};


  //////////////////////////////////////////////////////
  // inputs
  
  input_.resize(LastInputId+1); 
  input_[INPUT_TAGS]          = {{{COUNT, 9}, {TAG, 11}},                    RI};
  input_[DCT_FIFO_INPUT_TAGS] = {{{TAG, 11}},                                INIT_FIFO_DCT};
  input_[INPUT_SPIKES]        = {{{SYNAPSE_SIGN, 1}, {SYNAPSE_ADDRESS, 10}}, NEURON_INJECT};


  //////////////////////////////////////////////////////
  // outputs
  
  output_.resize(LastOutputId+1); // XXX this is kind of janky
  output_[PRE_FIFO_TAGS]   = {{{COUNT, 9}, {TAG, 11}},                     DUMP_PRE_FIFO};
  output_[POST_FIFO_TAGS0] = {{{COUNT, 9}, {TAG, 10}},                     DUMP_POST_FIFO0};
  output_[POST_FIFO_TAGS1] = {{{COUNT, 9}, {TAG, 10}},                     DUMP_POST_FIFO1};
  output_[OUTPUT_SPIKES]   = {{{NEURON_ADDRESS, 12}},                      NRNI};
  output_[OVERFLOW_TAGS0]  = {{{FIXED_1, 1}},                              OVFLW0};
  output_[OVERFLOW_TAGS1]  = {{{FIXED_1, 1}},                              OVFLW1};
  output_[ACC_OUTPUT_TAGS] = {{{COUNT, 9}, {TAG, 11}, {GLOBAL_ROUTE, 8}},  RO_ACC};
  output_[TAT_OUTPUT_TAGS] = {{{COUNT, 9}, {TAG, 11}, {GLOBAL_ROUTE, 12}}, RO_TAT};

  //////////////////////////////////////////////////////
  // misc
  
  misc_widths_.resize(LastMiscWidthId+1);
  misc_widths_[BD_INPUT] = 21;
  misc_widths_[BD_OUTPUT] = 34;

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

// clang-format on

RegId BDPars::DACSignalIdToDACRegisterId(DACSignalId id) const {
  assert(false && "not implemented");
  return DAC0;  // squelch compiler warning for now
}

unsigned int BDPars::WordFieldWidth(const WordStructure& word, WordFieldId field_id_to_match) const {
  for (auto& it : word) {
    WordFieldId field_id;
    unsigned int field_width;
    std::tie(field_id, field_width) = it;
    if (field_id == field_id_to_match) return field_width;
  }
  assert(false && "couldn't find field_id in word");
  return 0;
}

unsigned int BDPars::WordFieldWidth(MemId object, WordFieldId field_id, unsigned int subtype_idx) const {
  return WordFieldWidth(*Word(object, subtype_idx), field_id);
}

unsigned int BDPars::WordFieldWidth(MemWordId object, WordFieldId field_id) const {
  return WordFieldWidth(*Word(object), field_id);
}

unsigned int BDPars::WordFieldWidth(RegId object, WordFieldId field_id) const {
  return WordFieldWidth(*Word(object), field_id);
}

unsigned int BDPars::WordFieldWidth(InputId object, WordFieldId field_id) const {
  return WordFieldWidth(*Word(object), field_id);
}

unsigned int BDPars::WordFieldWidth(OutputId object, WordFieldId field_id) const {
  return WordFieldWidth(*Word(object), field_id);
}

uint64_t BDPars::ValueForSpecialFieldId(bdpars::WordFieldId field_id) {
  switch (field_id) {
    case UNUSED: {
      return 0;  // don't care
    }
    case FIXED_0: {
      return 0;
    }
    case FIXED_1: {
      return 1;
    }
    case FIXED_2: {
      return 2;
    }
    case FIXED_3: {
      return 3;
    }
    default: {
      assert(false && "no value supplied for a given field");
      return 0;  // suppresses compiler warning
    }
  }
}

bool BDPars::SpecialFieldValueMatches(bdpars::WordFieldId field_id, uint64_t val) {
  switch (field_id) {
    case UNUSED: {
      return true;  // don't care (although probably should be 0)
    }
    case FIXED_0: {
      return val == 0;
    }
    case FIXED_1: {
      return val == 1;
    }
    case FIXED_2: {
      return val == 2;
    }
    case FIXED_3: {
      return val == 3;
    }
    default: {
      return true;  // for normal fields, we don't care
    }
  }
}

}  // bdpars
}  // bddriver
}  // pystorm
