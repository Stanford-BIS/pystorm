#include "Encoder.h"

#include <cstdint>
#include <thread>
#include <unordered_map>
#include <vector>

#include "common/BDPars.h"
#include "common/MutexBuffer.h"
#include "common/binary_util.h"

#include <iostream>
using std::cout;
using std::endl;

namespace pystorm {
namespace bddriver {

void Encoder::RunOnce() {
  unsigned int num_popped = in_buf_->Pop(input_chunk_, input_chunk_size_, timeout_us_);
  Encode(input_chunk_, num_popped, output_chunks_[0]);
  bool success = false;
  while (!success && do_run_) {  // if killed, need to stop trying
    success = out_bufs_[0]->Push(output_chunks_[0], num_popped * bytesPerOutput, timeout_us_);
  }
}

void Encoder::Encode(const EncInput* inputs, unsigned int num_popped, EncOutput* outputs) const {
  for (unsigned int i = 0; i < num_popped; i++) {
    // unpack data
    // unsigned int core_id = inputs[i].core_id;
    uint8_t leaf_id = inputs[i].leaf_id;
    uint32_t payload     = inputs[i].payload;

    // look up route for this leaf_id_
    // XXX not doing anything with core_id

    // XXX this is where you would do something with the core id

    // encode horn
    uint32_t horn_encoded = EncodeHorn(leaf_id, payload);

    // XXX this is where you would encode the FPGA

    // unpack uint32_t w/ 21 bits into 3 uint8_ts
    uint32_t unpacked_bytes32[bytesPerOutput];
    unsigned int byte_widths[bytesPerOutput];
    for (unsigned int j = 0; j < bytesPerOutput; j++) {
      byte_widths[j] = 8;
    }

    Unpack32(horn_encoded, byte_widths, unpacked_bytes32, bytesPerOutput);

    for (unsigned int j = 0; j < bytesPerOutput; j++) {
      assert(unpacked_bytes32[j] < (1 << 8));
      outputs[i * bytesPerOutput + j] = static_cast<uint8_t>(unpacked_bytes32[j]);
    }

    //// XXX this is the sketchier (maybe faster) way. Have to know something about endianess
    // const uint8_t * bytes_for_USB = reinterpret_cast<const uint8_t *>(&horn_encoded)
    // for (unsigned int j = 0; j < bytesPerOutput; j++) {
    //  outputs[i * bytesPerOutput + j] = bytes_for_USB[j];
    //}
  }
}

inline uint32_t Encoder::EncodeHorn(uint8_t leaf_id, uint32_t payload) const {
  // msb <- lsb
  // [ leaf_id | payload ]

  uint32_t retval = payload | (leaf_id << 24);
  return retval;
}

}  // bddriver
}  // pystorm
