#ifndef DRIVERPARS_H
#define DRIVERPARS_H

#include <unordered_map>
#include <string>

namespace pystorm {
namespace bddriver {
namespace driverpars {

  constexpr unsigned int ms = 1000;

  // write sizes, in bytes
  constexpr unsigned int WRITE_BLOCK_SIZE = 512; 
  constexpr unsigned int WRITE_FIFO_DEPTH = 16 * 1024 * 4;
  constexpr unsigned int MAX_WRITE_SIZE = WRITE_FIFO_DEPTH / 2;

  // read sizes, in bytes
  constexpr unsigned int READ_BLOCK_SIZE = 512; 
  constexpr unsigned int READ_FIFO_DEPTH = 16 * 1024 * 4;
  constexpr unsigned int READ_SIZE = READ_FIFO_DEPTH / 2;

  constexpr unsigned int READ_LAG_WARNING_SIZE = 8 * READ_SIZE; // warning emitted when running 8 buffers behind or more
  constexpr unsigned int READ_FULL_WARNING_SIZE = static_cast<unsigned int>(READ_SIZE * .8);

  constexpr unsigned int BD_STATE_TRAFFIC_DRAIN_US = 
    1 * ms;  // timing assumption: this long after shutting off traffic, bd will be inactive

  constexpr unsigned int BDMODELCOMM_TRY_FOR_US = 1 * ms;
  constexpr unsigned int BDMODELCOMM_SLEEP_FOR_US = 1 * ms;

  constexpr unsigned int ENC_TIMEOUT_US = 1 * ms;
  constexpr unsigned int DEC_TIMEOUT_US = 1 * ms;

}  // driverpars
}  // bddriver
}  // pystorm

#endif
