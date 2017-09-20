#include "Encoder.h"

#include <cstdint>
#include <thread>
#include <unordered_map>
#include <vector>

#include "common/BDPars.h"
#include "common/BDWord.h"
#include "common/MutexBuffer.h"

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

  uint32_t* packed_outputs = reinterpret_cast<uint32_t*>(outputs);

  for (unsigned int i = 0; i < num_popped; i++) {
    // unpack data
    // unsigned int core_id = inputs[i].core_id;
    unsigned int FPGA_ep_code = inputs[i].FPGA_ep_code;
    uint32_t payload     = inputs[i].payload;

    // XXX this is where you would do something with the core id

    // pack into 32 bits
    // FPGA word format:
    //  MSB          LSB
    //    8b      24b
    // [ code | payload ]
    uint32_t FPGA_encoded = Pack<FPGAIO>({{FPGAIO::PAYLOAD, payload}, {FPGAIO::EP_CODE, FPGA_ep_code}});

    // XXX check endianness
    packed_outputs[i] = FPGA_encoded;
  }
}

}  // bddriver
}  // pystorm
