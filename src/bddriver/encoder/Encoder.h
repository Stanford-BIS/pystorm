#ifndef ENCODER_H
#define ENCODER_H

#include <cstdint>
#include <thread>
#include <utility>
#include <vector>

#include "common/BDPars.h"
#include "common/DriverTypes.h"
#include "common/MutexBuffer.h"
#include "common/Xcoder.h"

namespace pystorm {
namespace bddriver {

class Encoder : public Xcoder {
 public:
  const static unsigned int bytesPerOutput = 4;  

  Encoder(
      MutexBuffer<EncInput>* in_buf,
      MutexBuffer<EncOutput>* out_buf,
      const bdpars::BDPars * bd_pars,
      unsigned int timeout_us = 1000)
    : Xcoder(),
    timeout_us_(timeout_us),
    in_buf_(in_buf),
    out_buf_(out_buf),
    bd_pars_(bd_pars),
    last_HB_sent_at_(0) {};

  ~Encoder(){};

 private:
  const unsigned int timeout_us_;
  MutexBuffer<EncInput>* in_buf_;
  MutexBuffer<EncOutput>* out_buf_;
  const bdpars::BDPars * bd_pars_;
  unsigned int last_HB_sent_at_;

  void RunOnce();
  std::unique_ptr<std::vector<EncOutput>> Encode(const std::unique_ptr<std::vector<EncInput>> inputs);
};

}  // bddriver
}  // pystorm

#endif
