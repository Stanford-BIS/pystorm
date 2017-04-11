#ifndef HWLOC_H
#define HWLOC_H

#include <string>

namespace pystorm {
namespace bddriver {

struct HWLoc {
  unsigned int core_id_;
  unsigned int leaf_idx_;
};

} // bddriver
} // pystorm

#endif
