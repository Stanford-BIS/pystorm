#include "BDPars.h"

#include <vector>
#include <string>
#include <unordered_map>

#include "common/binary_util.h"

namespace pystorm {
namespace bddriver {

BDPars::BDPars() 
{
  // XXX WIP

  //////////////////////////////////////////////////////
  // Funnel/Horn

  //                              {route, route_len, data width, serialization, serialized data width, description}
  horn_["NeuronInject"]         = {7  , 4  , 11 , 1  , 11 , "direct spike injection to neuron array"};
  horn_["RI"]                   = {0  , 1  , 20 , 1  , 20 , "main tag input to FIFO"};
  horn_["PROG_AMMM"]            = {57 , 6  , 42 , 4  , 11 , "AM/MM programming/diagnostic port"};
  horn_["PROG_PAT"]             = {41 , 6  , 27 , 4  , 7  , "PAT programming/diagnostic port"};
  horn_["PROG_TAT[0]"]          = {65 , 7  , 31 , 4  , 8  , "TAT 0 programming/diagnostic port"};
  horn_["PROG_TAT[1]"]          = {97 , 7  , 31 , 4  , 8  , "TAT 1 programming/diagnostic port"};
  horn_["INIT_FIFO_DCT"]        = {49 , 7  , 11 , 1  , 11 , "inserts a tag into the DCT side of the FIFO with ct = 1, needed to clean initial FIFO state"};
  horn_["INIT_FIFO_HT"]         = {17 , 8  , 1  , 1  , 1  , "trigger sets FIFO head/tail register to empty state"};
  horn_["TOGGLE_PRE_FIFO"]      = {145, 8  , 2  , 1  , 2  , "toggles data/dump traffic for FIFO input"};
  horn_["TOGGLE_POST_FIFO[0]"]  = {81 , 8  , 2  , 1  , 2  , "toggles data/dump traffic for FIFO tag class 0 output"};
  horn_["TOGGLE_POST_FIFO[1]"]  = {209, 8  , 2  , 1  , 2  , "toggles data/dump traffic for FIFO tag class 1 output"};
  horn_["NeuronDumpToggle"]     = {15 , 4  , 2  , 1  , 2  , "toggles data/dump traffic for neuron array output"};
  horn_["NeuronConfig"]         = {3  , 3  , 18 , 1  , 18 , "programming input for neuron array tile SRAM"};
  horn_["DAC[0]"]               = {13 , 8  , 11 , 1  , 11 , "DIFF_G DAC bias value"};
  horn_["DAC[1]"]               = {141, 8  , 11 , 1  , 11 , "DIFF_R DAC bias value"};
  horn_["DAC[2]"]               = {77 , 8  , 11 , 1  , 11 , "SOMA_OFFSET DAC bias value"};
  horn_["DAC[3]"]               = {205, 8  , 11 , 1  , 11 , "SYN_LK DAC bias value"};
  horn_["DAC[4]"]               = {45 , 8  , 11 , 1  , 11 , "SYN_DC DAC bias value"};
  horn_["DAC[5]"]               = {173, 8  , 11 , 1  , 11 , "SYN_PD DAC bias value"};
  horn_["DAC[6]"]               = {109, 8  , 11 , 1  , 11 , "ADC_BIAS_2 DAC bias value"};
  horn_["DAC[7]"]               = {237, 8  , 11 , 1  , 11 , "ADC_BIAS_1 DAC bias value"};
  horn_["DAC[8]"]               = {29 , 8  , 11 , 1  , 11 , "SOMA_REF DAC bias value"};
  horn_["DAC[9]"]               = {157, 8  , 11 , 1  , 11 , "SOMA_EXC DAC bias value"};
  horn_["DAC[10]"]              = {93 , 7  , 11 , 1  , 11 , "SOMA_INH DAC bias value"};
  horn_["DAC[11]"]              = {61 , 7  , 11 , 1  , 11 , "SYN_PU DAC bias value"};
  horn_["DAC[12]"]              = {125, 7  , 11 , 1  , 11 , "UNUSED (ghost DAC)"};
  horn_["ADC"]                  = {5  , 4  , 3  , 1  , 3  , "ADC small/large current enable, output enable"};
  horn_["DELAY[0]"]             = {1  , 7  , 8  , 1  , 8  , "FIFO:DCT delay line config"};
  horn_["DELAY[1]"]             = {33 , 7  , 8  , 1  , 8  , "FIFO:PG delay line config"};
  horn_["DELAY[2]"]             = {113, 8  , 8  , 1  , 8  , "TAT 0 delay line config"};
  horn_["DELAY[3]"]             = {241, 8  , 8  , 1  , 8  , "TAT 1 delay line config"};
  horn_["DELAY[4]"]             = {9  , 6  , 8  , 1  , 8  , "PAT delay line config"};
  horn_["DELAY[5]"]             = {25 , 7  , 8  , 1  , 8  , "MM delay line config"};
  horn_["DELAY[6]"]             = {89 , 7  , 8  , 1  , 8  , "AM delay line config"};

  //                              {route, route_len, data width, serialization, serialized data width, description}
  funnel_["RO_ACC"]             = {2  , 2  , 28 , 1  , 28 , "tag output from accumulator"};
  funnel_["RO_TAT"]             = {0  , 2  , 32 , 1  , 32 , "tag output from TAT"};
  funnel_["NRNI"]               = {3  , 2  , 12 , 1  , 12 , "copy of traffic exiting neuron array"};
  funnel_["DUMP_AM"]            = {5  , 6  , 38 , 2  , 19 , "AM diagnostic read output"};
  funnel_["DUMP_MM"]            = {37 , 6  , 8  , 1  , 8  , "MM diagnostic read output"};
  funnel_["DUMP_PAT"]           = {21 , 5  , 20 , 1  , 20 , "PAT diagnostic read output"};
  funnel_["DUMP_TAT[0]"]        = {1  , 4  , 29 , 1  , 29 , "TAT 0 diagnostic read output"};
  funnel_["DUMP_TAT[1]"]        = {9  , 4  , 29 , 1  , 29 , "TAT 1 diagnostic read output"};
  funnel_["DUMP_PRE_FIFO"]      = {45 , 6  , 20 , 1  , 20 , "copy of traffic entering FIFO"};
  funnel_["DUMP_POST_FIFO[0]"]  = {29 , 6  , 19 , 1  , 19 , "copy of tag class 0 traffic exiting FIFO"};
  funnel_["DUMP_POST_FIFO[1]"]  = {61 , 6  , 19 , 1  , 19 , "copy of tag class 1 traffic exiting FIFO"};
  funnel_["OVFLW[0]"]           = {13 , 7  , 1  , 1  , 1  , "class 0 FIFO overflow warning"};
  funnel_["OVFLW[1]"]           = {77 , 7  , 1  , 1  , 1  , "class 1 FIFO overflow warning"};

  //////////////////////////////////////////////////////
  // Registers
  
  WordStructure toggle_word = {{"data flow on/off", 1}, {"dump flow on/off", 1}};
  WordStructure DAC_word = {{"DAC to ADC conn", 1}, {"DAC value", 10}};
  WordStructure DELAY_word = {{"read delay", 4}, {"write delay", 4}};
  
  reg_["ADC"]                 = {{{"ADC[0] small/large current", 1}, {"ADC[1] small/large current", 1}, {"ADC output enable", 1}}};
  reg_["TOGGLE_PRE_FIFO"]     = {{toggle_word}};
  reg_["TOGGLE_POST_FIFO[0]"] = {{toggle_word}};
  reg_["TOGGLE_POST_FIFO[1]"] = {{toggle_word}};
  reg_["NeuronDumpToggle"]    = {{toggle_word}};
  reg_["DAC[0]"]              = {{DAC_word}};
  reg_["DAC[1]"]              = {{DAC_word}};
  reg_["DAC[2]"]              = {{DAC_word}};
  reg_["DAC[3]"]              = {{DAC_word}};
  reg_["DAC[4]"]              = {{DAC_word}};
  reg_["DAC[5]"]              = {{DAC_word}};
  reg_["DAC[6]"]              = {{DAC_word}};
  reg_["DAC[7]"]              = {{DAC_word}};
  reg_["DAC[8]"]              = {{DAC_word}};
  reg_["DAC[9]"]              = {{DAC_word}};
  reg_["DAC[10]"]             = {{DAC_word}};
  reg_["DAC[11]"]             = {{DAC_word}};
  reg_["DAC[12]"]             = {{DAC_word}};
  reg_["DELAY[0]"]            = {{DELAY_word}};
  reg_["DELAY[1]"]            = {{DELAY_word}};
  reg_["DELAY[2]"]            = {{DELAY_word}};
  reg_["DELAY[3]"]            = {{DELAY_word}};
  reg_["DELAY[4]"]            = {{DELAY_word}};
  reg_["DELAY[5]"]            = {{DELAY_word}};
  reg_["DELAY[6]"]            = {{DELAY_word}};

  //////////////////////////////////////////////////////
  // Memories
  
  std::vector<WordStructure> AM_words  = {{{"value", 15}, 
                                           {"threshold", 3}, 
                                           {"stop", 1}, 
                                           {"next address", 19}}};
  std::vector<WordStructure> MM_words  = {{{"weight", 8}}};
  std::vector<WordStructure> PAT_words = {{{"AM address", 10}, 
                                           {"MM address low bits", 8}, 
                                           {"MM address base high bits", 2}}};
  std::vector<WordStructure> TAT_words = {{{"AM address", 10}, // acc type
                                           {"MM address", 16}},
                                          {{"synapse id 0", 11}, // spike type
                                           {"synapse sign 0", 1}, 
                                           {"synapse id 1", 11}, 
                                           {"synapse sign 1", 1}, 
                                           {"unused", 2}},
                                          {{"tag", 11}, // fanout type
                                           {"global route", 12}, 
                                           {"unused",3}}};

  mem_["AM"]  = {1024,    AM_words};
  mem_["MM"]  = {64*1024, MM_words};
  mem_["TAT"] = {1024,    TAT_words};
  mem_["PAT"] = {64,      PAT_words};

  mem_prog_words_["PAT write"]             = {{"address", 6}, {"FIXED=0", 2}, {"data", 20}};
  mem_prog_words_["PAT read"]              = {{"address", 6}, {"FIXED=1", 2}, {"unused", 20}};

  mem_prog_words_["TAT address"]           = {{"FIXED=0", 2}, {"address", 10}, {"unused", 19}};
  mem_prog_words_["TAT write+increment"]   = {{"FIXED=1", 2}, {"data", 29}};
  mem_prog_words_["TAT read+increment"]    = {{"FIXED=2", 2}, {"unused", 29}};

  mem_prog_words_["MM address"]            = {{"FIXED=0", 2}, {"address", 16}, {"unused", 22}};
  mem_prog_words_["MM write+increment"]    = {{"FIXED=1", 2}, {"data", 8}, {"unused", 30    }};
  mem_prog_words_["MM read+increment"]     = {{"FIXED=2", 2}, {"unused", 38}};
  mem_prog_words_["AM address"]            = {{"FIXED=0", 2}, {"address", 10}, {"unused", 28}};
  mem_prog_words_["AM read+write"]         = {{"FIXED=1", 2}, {"data", 38}};
  mem_prog_words_["AM increment"]          = {{"FIXED=2", 2}, {"unused", 38}};

  mem_prog_words_["AM encapsulation"] = {{"FIXED=0", 1}, {"payload", 40}, {"stop", 1}};
  mem_prog_words_["MM encapsulation"] = {{"FIXED=1", 1}, {"payload", 40}, {"stop", 1}};


  //////////////////////////////////////////////////////
  // inputs
  
  io_input_width_ = 21;

  input_["RI"]            = {{"ct", 9}, {"tag", 11}};
  input_["INIT_FIFO_DCT"] = {{"tag", 11}};
  input_["NeuronInject"]  = {{"sign", 1}, {"synapse index", 10}};

  //////////////////////////////////////////////////////
  // outputs
  
  io_output_width_ = 34;

  output_["DUMP_PRE_FIFO"]     = {{"ct", 9}, {"tag", 11}};
  output_["DUMP_POST_FIFO[0]"] = {{"ct", 9}, {"tag", 10}};
  output_["DUMP_POST_FIFO[1]"] = {{"ct", 9}, {"tag", 10}};
  output_["NRNI"]              = {{"neuron id", 12}};
  output_["OVFLW[0]"]          = {{"notification (always == b1)", 1}};
  output_["OVFLW[1]"]          = {{"notification (always == b1)", 1}};
  output_["RO_ACC"]            = {{"ct", 9}, {"tag", 11}, {"global route", 8}};
  output_["RO_TAT"]            = {{"ct", 9}, {"tag", 11}, {"global route", 12}};

  //////////////////////////////////////////////////////
  // Postprocessing

  // map funnel/horn leaf names to indices used internally
  unsigned int idx = 0;
  for (auto& it : horn_) {
    std::string horn_leaf_name = it.first;
    horn_leaf_name_to_idx_[horn_leaf_name] = idx;
  }

  for (auto& it : funnel_) {
    std::string funnel_leaf_name = it.first;
    funnel_leaf_name_to_idx_[funnel_leaf_name] = idx;
  }
  
  // create direct-mapped tables used in encoding/decoding
  for (auto& it : horn_) {
    LeafInfo leaf_info = it.second;
    horn_routes_.push_back(std::make_pair(leaf_info.route_val, leaf_info.route_len));
  }

  for (auto& it : funnel_) {
    LeafInfo leaf_info = it.second;
    funnel_routes_.push_back(std::make_pair(leaf_info.route_val, leaf_info.route_len));
  }

}

unsigned int BDPars::Width(const std::string& object) const
{
  // try all the different object types
  if (funnel_.count(object) > 0) {
    return funnel_.at(object).data_width;
  } else if (horn_.count(object) > 0) {
    return horn_.at(object).data_width;
  } else {
    assert(false && "couldn't find desired object");
  }
}

const WordStructure * BDPars::Word(const std::string & object, unsigned int subtype_idx) const
{
  if (mem_.count(object) > 0) {
    return &(mem_.at(object).word_structures.at(subtype_idx));
  } else if (mem_prog_words_.count(object) > 0) {
    return &(mem_prog_words_.at(object));
  } else if (reg_.count(object) > 0) {
    return &(reg_.at(object).word_structure);
  } else if (input_.count(object) > 0) {
    return &(input_.at(object));
  } else if (output_.count(object) > 0) {
    return &(output_.at(object));
  } else {
    assert(false && "couldn't find desired object");
  }
}


} // bddriver
} // pystorm
