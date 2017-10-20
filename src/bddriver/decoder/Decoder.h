#ifndef DECODER_H
#define DECODER_H

#include <cstdint>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "common/BDPars.h"
#include "common/DriverTypes.h"
#include "common/MutexBuffer.h"
#include "common/Xcoder.h"
#include "common/vector_util.h"

namespace pystorm {
namespace bddriver {

class Decoder : public Xcoder {

 public:

  constexpr static unsigned int bytesPerInput = 4; 

  Decoder(
      MutexBuffer<DecInput> *in_buf,
      const std::unordered_map<uint8_t, MutexBuffer<DecOutput> *> &out_bufs,
      const bdpars::BDPars * bd_pars,
      unsigned int timeout_us = 1000)
    : Xcoder(), 
    timeout_us_(timeout_us), 
    in_buf_(in_buf),
    out_bufs_(out_bufs),
    bd_pars_(bd_pars),
    last_HB_recvd_(0),
    next_HB_significance_(NextHBSignificance::LSB),
    deserializer_(new VectorDeserializer<DecInput>(bytesPerInput)) {};

  ~Decoder() { delete deserializer_; };

 private:

  enum class NextHBSignificance {LSB, MSB};

  const unsigned int timeout_us_;
  MutexBuffer<DecInput> * in_buf_;
  std::unordered_map<uint8_t, MutexBuffer<DecOutput> *> out_bufs_;
  const bdpars::BDPars * bd_pars_;

  BDTime last_HB_recvd_;

  // because of the "push" output problem, we have to shift how we label times by
  // two words: the time that event i actually happened is the time for event i - 2
  BDTime word_i_min_2_time_ = 0;
  BDTime word_i_min_1_time_ = 0;

  NextHBSignificance next_HB_significance_; // whether the next upstream HB has LSBs or MSBs

  VectorDeserializer<DecInput> * deserializer_;

  void RunOnce();
  std::vector<uint32_t> PackBytes(std::unique_ptr<std::vector<DecInput>> input);
  std::unordered_map<uint8_t, std::unique_ptr<std::vector<DecOutput>>> Decode(std::unique_ptr<std::vector<DecInput>> input);

};

}  // bddriver
}  // pystorm

#endif
