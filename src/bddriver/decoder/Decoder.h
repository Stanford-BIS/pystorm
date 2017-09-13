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

namespace pystorm {
namespace bddriver {

class Decoder : public Xcoder<DecInput, DecOutput> {
 public:
  const static unsigned int bytesPerInput = 4;  // FPGA_BD_output is 32 bits, fits in 4 bytes

  Decoder(
      const bdpars::BDPars *pars,
      MutexBuffer<DecInput> *in_buf,
      const std::vector<MutexBuffer<DecOutput> *> &out_bufs,
      unsigned int chunk_size,
      unsigned int timeout_us = 1000);
  ~Decoder();

  // for testing
  // unsigned int num_processed_;

 private:
  void RunOnce();
  std::pair<uint8_t, uint32_t> DecodeFunnel(uint32_t input) const;
  void Decode(
      const DecInput *inputs,
      unsigned int num_popped,
      std::vector<DecOutput *> *outputs,
      std::vector<unsigned int> *num_pushed_to_each) const;
};

}  // bddriver
}  // pystorm

#endif
