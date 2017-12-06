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

  constexpr static unsigned int READ_SIZE = 512 * 16;
  constexpr static unsigned int READ_BLOCK_SIZE = 1024; // XXX shouldn't hardcode this, should get from Comm
  constexpr static unsigned int BYTES_PER_WORD = 4;
  constexpr static unsigned int bytesPerInput = BYTES_PER_WORD;

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
    last_HB_LSB_recvd_(0),
    curr_HB_recvd_(0),
    last_HB_recvd_(0) {};

  ~Decoder() {};

 private:

  const unsigned int timeout_us_;
  MutexBuffer<DecInput> * in_buf_;
  std::unordered_map<uint8_t, MutexBuffer<DecOutput> *> out_bufs_;
  const bdpars::BDPars * bd_pars_;

  uint32_t last_HB_LSB_recvd_;
  BDTime curr_HB_recvd_;
  BDTime last_HB_recvd_;

  // because of the "push" output problem, we have to shift how we label times by
  // two words: the time that event i actually happened is the time for event i - 2
  BDTime word_i_min_2_time_ = 0;
  BDTime word_i_min_1_time_ = 0;


  void RunOnce();
  std::unordered_map<uint8_t, std::unique_ptr<std::vector<DecOutput>>> Decode(std::unique_ptr<std::vector<DecInput>> input);

};

}  // bddriver
}  // pystorm

#endif
