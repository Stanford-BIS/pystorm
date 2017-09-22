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
  // we may time out for the Pop, (which can block indefinitely), giving us a chance to be killed
  std::unique_ptr<std::vector<EncInput>> popped_vect = in_buf_->Pop(timeout_us_);
  if (popped_vect->size() > 0) {
    std::unique_ptr<std::vector<EncOutput>> to_push_vect Encode(std::move(popped_vect));
    out_bufs_[0]->Push(std::move(to_push_vect)); // push can't block indefinitely
  }
}

std::unique_ptr<std::vector<EncOutput>> Encoder::Encode(const std::unique_ptr<std::vector<EncInput>> inputs) const {

  std::unique_ptr<std::vector<EncOutput>> output (new std::vector<EncOutput>);

  for (auto& it : *inputs) {
    // unpack data
    unsigned int core_id      = it.core_id;
    unsigned int FPGA_ep_code = it.FPGA_ep_code;
    uint32_t     payload      = it.payload;
    BDTime       time         = it.time;

    (void)core_id; // XXX this is where you would do something with core_id
    (void)time;    // XXX this is where you would do something with time

    // pack into 32 bits
    // FPGA word format:
    //  MSB          LSB
    //    8b      24b
    // [ code | payload ]
    uint32_t FPGA_encoded = PackWord<FPGAIO>({{FPGAIO::PAYLOAD, payload}, {FPGAIO::EP_CODE, FPGA_ep_code}});

    // serialize to bytes 
    uint8_t b0 = GetField<FPGABYTES>(FPGA_encoded, FPGABYTES::B0);
    uint8_t b1 = GetField<FPGABYTES>(FPGA_encoded, FPGABYTES::B1);
    uint8_t b2 = GetField<FPGABYTES>(FPGA_encoded, FPGABYTES::B2);
    uint8_t b3 = GetField<FPGABYTES>(FPGA_encoded, FPGABYTES::B3);
    output->push_back(b0);
    output->push_back(b1);
    output->push_back(b2);
    output->push_back(b3);
  }

  return output;
}

}  // bddriver
}  // pystorm
