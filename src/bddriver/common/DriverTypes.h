#ifndef DRIVERTYPES_H
#define DRIVERTYPES_H

#include <cstdint>
#include <map>

#include "BDPars.h"

namespace pystorm {
namespace bddriver {

////////////////////////////////////////
// decoder/encoder

// decoder
struct DecOutput {
  uint32_t payload;
  unsigned int core_id;
  unsigned int time_epoch;
};
typedef uint8_t DecInput;

// encoder
struct EncInput {
  unsigned int core_id;
  uint8_t leaf_id;
  uint32_t payload;
  // XXX should probably have time
};
typedef uint8_t EncOutput;

typedef unsigned int BDTime;

}  // bddriver
}  // pystorm

#endif
