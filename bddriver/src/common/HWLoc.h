#ifndef HWLOC_H
#define HWLOC_H

#include <string>

namespace pystorm {
namespace bddriver {

struct HWLoc {
  unsigned int core_id;
  unsigned int leaf_idx;
};

} // bddriver
} // pystorm

#endif
