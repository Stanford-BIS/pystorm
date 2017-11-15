#include "Decoder.h"

#include <cstdint>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "common/DriverTypes.h"
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
    for (auto& it : to_push_vects) {
      uint8_t ep_code = it.first;
      std::unique_ptr<std::vector<DecOutput>> &vvect = it.second;
      out_bufs_.at(ep_code)->Push(std::move(vvect));
    }

  }
}

// pack inputs into 32-bit FPGA words, using remainder
std::vector<uint32_t> Decoder::PackBytes(std::unique_ptr<std::vector<DecInput>> input) {

  // load deserializer with new input
  deserializer_->NewInput(std::move(input));

  std::vector<uint32_t> packed;

  std::vector<uint8_t> deserialized; // continuosly write into here

  deserializer_->GetOneOutput(&deserialized);
  while (deserialized.size() > 0) {
    packed.push_back(PackWord<FPGABYTES>(
         {{FPGABYTES::B0, deserialized.at(0)}, 
          {FPGABYTES::B1, deserialized.at(1)}, 
          {FPGABYTES::B2, deserialized.at(2)}, 
          {FPGABYTES::B3, deserialized.at(3)}}));

    deserializer_->GetOneOutput(&deserialized);
  }

  return packed;
}

std::unordered_map<uint8_t, std::unique_ptr<std::vector<DecOutput>>> Decoder::Decode(std::unique_ptr<std::vector<DecInput>> input) {

  // pack inputs into 32-bit FPGA words, starting with the remainder
  std::vector<uint32_t> inputs_packed = PackBytes(std::move(input));

  std::unordered_map<uint8_t, std::unique_ptr<std::vector<DecOutput>>> outputs;

  for (auto& it : inputs_packed) {

    // XXX this is where you would do something with the core id

    // decode EP_code
    unsigned int ep_code = GetField<FPGAIO>(it, FPGAIO::EP_CODE);
    uint32_t payload     = GetField<FPGAIO>(it, FPGAIO::PAYLOAD);

    // if it's a heartbeat, set last_HB_recvd
    if (ep_code == bd_pars_->UpEPCodeFor(bdpars::FPGAOutputEP::UPSTREAM_HB)) {
      // unpack current time MSB and LSB
      uint64_t curr_HB_msb = GetField(last_HB_recvd_, TWOFPGAPAYLOADS::MSB);
      uint64_t curr_HB_lsb = GetField(last_HB_recvd_, TWOFPGAPAYLOADS::LSB);

      // update last_HB_recvd_ LSB
      if (next_HB_significance_ == NextHBSignificance::LSB) {
        last_HB_recvd_ = PackWord<TWOFPGAPAYLOADS>(
            {{TWOFPGAPAYLOADS::MSB, curr_HB_msb}, 
             {TWOFPGAPAYLOADS::LSB, payload}});
        next_HB_significance_ = NextHBSignificance::MSB;

      // update last_HB_recvd_ MSB
      } else if (next_HB_significance_ == NextHBSignificance::MSB) {
        last_HB_recvd_ = PackWord<TWOFPGAPAYLOADS>(
            {{TWOFPGAPAYLOADS::MSB, payload},
             {TWOFPGAPAYLOADS::LSB, curr_HB_lsb}});
        next_HB_significance_ = NextHBSignificance::LSB;
      } else {
        assert(false && "something wrong with next_HB_significance_ enum");
      }
      //cout << "got HB: " << payload << " curr_HB_ = " << last_HB_recvd_ << endl;
      // we send the HBs to the driver too, so it knows the time
    }

    // ignore nop
    if (ep_code == bd_pars_->UpEPCodeFor(bdpars::FPGAOutputEP::NOP)) {
      // do nothing
      
    // otherwise, forward to Driver
    } else {
      //cout << "decoder got something that wasn't a HB" << endl;
      //cout << "  ep# = " << ep_code << endl;

      DecOutput to_push;
      to_push.payload = payload;

      // update times for "push" output problem
      // edit: I think we're actually only 1 word behind
      to_push.time       = word_i_min_1_time_;
      //word_i_min_2_time_ = word_i_min_1_time_;
      word_i_min_1_time_ = last_HB_recvd_;

      if (outputs.count(ep_code) == 0) {
        outputs[ep_code] = std::make_unique<std::vector<DecOutput>>();
      }
      outputs.at(ep_code)->push_back(to_push);
    }
  }

  return outputs;
}

}  // bddriver
}  // pystorm
