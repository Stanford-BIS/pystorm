namespace pystorm {
namespace bddriver {

#include "bddriver/common/BDPars.h"

#include <string>
#include <unordered_map>
#include <pair>

BDPars::BDPars(std::string bd_yaml) 
{
  // XXX WIP: this is just for testing
  // this function is supposed to parse the yaml and fill in the data members
  leaf_routes_["someleaf"] = Binary(1, 1);
}

} // bddriver
} // pystorm
