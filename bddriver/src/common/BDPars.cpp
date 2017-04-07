#include "BDPars.h"

#include <string>
#include <unordered_map>

namespace pystorm {
namespace bddriver {

BDPars::BDPars(std::string bd_yaml) 
{
  // XXX WIP: this is just for testing
  // this function is supposed to parse the yaml and fill in the data members
  horn_routes_["softleaf"] = Binary(1, 1);
  funnel_routes_["softleaf"] = Binary(1, 1);

  // XXX document these names in the wiki
  widths_["BD_input"] = 21;
  widths_["BD_output"] = 34;
}

} // bddriver
} // pystorm
