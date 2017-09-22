#include "Decoder.h"

#include <cstdint>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "common/BDPars.h"
#include "common/BDWord.h"
#include "common/MutexBuffer.h"
#include "common/vector_util.h" // VectorDeserializer

#include <iostream>
using std::cout;
using std::endl;

namespace pystorm {
namespace bddriver {

void Decoder::RunOnce() {
  // we may time out for the Pop, (which can block indefinitely), giving us a chance to be killed
  std::unique_ptr<std::vector<DecInput>> popped_vect = in_buf_->Pop(timeout_us_);
  if (popped_vect->size() > 0) {
    std::unordered_map<uint8_t, std::unique_ptr<std::vector<DecOutput>>> to_push_vects = Decode(std::move(popped_vect));

    // push to each output vector
    assert(to_push_vects.size() == out_bufs_.size());
    for (unsigned int i = 0; i < out_bufs_.size(); i++) {
      out_bufs_[i]->Push(std::move(to_push_vects[i]));
    }

  }
}

// pack inputs into 32-bit FPGA words, using remainder
std::vector<uint32_t> Decoder::PackBytes(std::unique_ptr<const std::vector<DecInput>> input) {

  // load deserializer with new input
  deserializer_.NewInput(input.get());

  std::vector<uint32_t> packed;

  std::vector<uint8_t> deserialized; // continuosly write into here

  deserializer_.GetOneOutput(&deserialized);
  while (deserialized.size() > 0) {
    packed.push_back(PackWord<FPGABYTES>(
         {{FPGABYTES::B0, deserialized.at(0)}, 
          {FPGABYTES::B1, deserialized.at(1)}, 
          {FPGABYTES::B2, deserialized.at(2)}, 
          {FPGABYTES::B3, deserialized.at(3)}}));

    deserializer_.GetOneOutput(&deserialized);
  }

  return packed;
}

std::unordered_map<uint8_t, std::unique_ptr<std::vector<DecOutput>>> Decoder::Decode(std::unique_ptr<const std::vector<DecInput>> input) {

  // pack inputs into 32-bit FPGA words, starting with the remainder
  std::vector<uint32_t> inputs_packed = PackBytes(std::move(input));

  std::unordered_map<uint8_t, std::unique_ptr<std::vector<DecOutput>>> outputs;

  for (auto& it : inputs_packed) {

    // XXX this is where you woudl do something with time
    BDTime time = 0;

    // XXX this is where you would do something with the core id

    // decode EP_code
    unsigned int ep_code = GetField<FPGAIO>(it, FPGAIO::EP_CODE);
    uint32_t payload     = GetField<FPGAIO>(it, FPGAIO::PAYLOAD);

    DecOutput to_push;
    to_push.payload = payload;
    to_push.time    = time;

    if (outputs.count(ep_code) == 0) {
      outputs.at(ep_code) = std::unique_ptr<std::vector<DecOutput>>(new std::vector<DecOutput>);
    }
    outputs.at(ep_code)->push_back(to_push);
  }
  return outputs;
}

}  // bddriver
}  // pystorm
