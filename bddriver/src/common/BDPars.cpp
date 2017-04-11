#include "BDPars.h"

#include <vector>
#include <string>
#include <unordered_map>

namespace pystorm {
namespace bddriver {

BDPars::BDPars() 
{
  // XXX WIP
  
  // XXX document these names in the wiki
  widths_["BD_input"] = 21;
  widths_["BD_output"] = 34;

  // XXX this is harcoded for now
  // come up with a more permanent format for declaring the routing table
  // and extracting this information
  horn_leaf_name_to_idx_["RI"] = 0;
  horn_leaf_name_to_idx_["NeuronInject"] = 1;
  horn_leaf_name_to_idx_["PROG_PAT"] = 2;

  funnel_leaf_name_to_idx_["NRNI"] = 0;
  funnel_leaf_name_to_idx_["RO_ACC"] = 1;
  funnel_leaf_name_to_idx_["RO_TAT"] = 2;
  funnel_leaf_name_to_idx_["DUMP_PAT"] = 3;
  
  // XXX these numbers might be wrong, check python code
  horn_routes_ = {
    std::make_pair(0, 1), // RI
    std::make_pair(7, 4), // NeuronInject
    std::make_pair(41, 6) // PROG_PAT
  };

  funnel_routes_ = {
    std::make_pair(3, 2), // NRNI
    std::make_pair(2, 2), // RO_ACC
    std::make_pair(0, 2), // RO_TAT
    std::make_pair(21, 5), // DUMP_PAT
  };

}

} // bddriver
} // pystorm
