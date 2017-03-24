#include "common/HWLoc.h"

#include <string>

namespace pystorm {
namespace bddriver {

HWLoc::HWLoc(unsigned int chip_id, const std::string& leaf_name)
{ 
  chip_id_ = chip_id; 
  leaf_name_ = leaf_name;
}

} // bddriver
} // pystorm
