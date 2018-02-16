#ifndef DRIVERTYPES_H
#define DRIVERTYPES_H

#include <cstdint>
#include <map>

#include "BDPars.h"

namespace pystorm {
namespace bddriver {

////////////////////////////////////////
// decoder/encoder

typedef uint64_t BDTime;

// decoder
struct DecOutput {
  uint32_t     payload;
  BDTime       time;
};
typedef uint8_t DecInput;

// encoder
struct EncInput {

  // if the encoder gets this, it finishes the block it's on
  const static unsigned int kFlushCode = UINT8_MAX;

  unsigned int core_id;
  uint8_t      FPGA_ep_code;
  uint32_t     payload;
  BDTime       time;
  unsigned int sequence_num; // used in priority queue, encoder doesn't need this
};
typedef uint8_t EncOutput;

// used for the priority queue
inline bool operator<(const EncInput & e1, const EncInput & e2) { 
  if (e1.time == e2.time) {
    return e1.sequence_num < e2.sequence_num;
  } else {
    return e1.time < e2.time; 
  }
}

}  // bddriver
}  // pystorm

#endif
