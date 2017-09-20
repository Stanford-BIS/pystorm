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

class Encoder : public Xcoder<EncInput, EncOutput> {
 public:
  const static unsigned int bytesPerOutput = 4;  

  Encoder(
      const bdpars::BDPars* pars,
      MutexBuffer<EncInput>* in_buf,
      MutexBuffer<EncOutput>* out_buf,
      unsigned int chunk_size,
      unsigned int timeout_us = 1000)
      : Xcoder(
            pars,
            in_buf,
            {out_buf},
            chunk_size,
            chunk_size * bytesPerOutput,
            timeout_us){};  // call base constructor only
  ~Encoder(){};

 private:
  void RunOnce();
  void Encode(const EncInput* inputs, unsigned int num_popped, EncOutput* outputs) const;
};

}  // bddriver
}  // pystorm

#endif
