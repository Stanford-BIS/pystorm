#ifndef DRIVERTYPES_H
#define DRIVERTYPES_H

#include <cstdint>
#include <map>

#include "BDPars.h"

namespace pystorm {
namespace bddriver {

////////////////////////////////////////
// decoder/encoder

typedef unsigned int BDTime;

// decoder
struct DecOutput {
     uint32_t     payload;
     BDTime       time;
};
typedef uint8_t DecInput;

// encoder
struct EncInput {
     unsigned int core_id;
     uint8_t      FPGA_ep_code;
     uint32_t     payload;
     BDTime       time;
};
typedef uint8_t EncOutput;

}  // bddriver
}  // pystorm

#endif
