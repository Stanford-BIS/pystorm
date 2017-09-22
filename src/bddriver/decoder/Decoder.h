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
      unsigned int timeout_us = 1000)
    : Xcoder(), 
    timeout_us_(timeout_us), 
    in_buf_(in_buf),
    out_bufs_(out_bufs),
    deserializer_(VectorDeserializer<DecInput>(bytesPerInput)) {};

  ~Decoder();

 private:

  const unsigned int timeout_us_;
  MutexBuffer<DecInput> * in_buf_;
  std::unordered_map<uint8_t, MutexBuffer<DecOutput> *> out_bufs_;

  VectorDeserializer<DecInput> deserializer_;

  void RunOnce();
  std::vector<uint32_t> PackBytes(std::unique_ptr<const std::vector<DecInput>> input);
  std::unordered_map<uint8_t, std::unique_ptr<std::vector<DecOutput>>> Decode(std::unique_ptr<const std::vector<DecInput>> input);

};

}  // bddriver
}  // pystorm

#endif
