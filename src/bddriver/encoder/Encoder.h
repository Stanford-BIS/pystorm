#ifndef ENCODER_H
#define ENCODER_H

#include <cstdint>
#include <thread>
#include <utility>
#include <vector>
#include <chrono>

#include "common/BDPars.h"
#include "common/DriverPars.h"
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
    last_HB_sent_at_(0),
    output_block_(std::make_unique<std::vector<EncOutput>>()) {};

  ~Encoder(){};

  // driver calls this. NOT THREADSAFE. MUST STOP ENCODER BEFORE CALLING!
  void ResetBaseTime(); 

 private:

  std::deque<std::pair<unsigned int, BDTime>> block_sizes_times_outstanding_;
  unsigned int total_outstanding_ = 0;

  std::chrono::high_resolution_clock::time_point base_time_ = 
    std::chrono::high_resolution_clock::now(); 

  const unsigned int timeout_us_;
  MutexBuffer<EncInput>* in_buf_;
  MutexBuffer<EncOutput>* out_buf_;
  const bdpars::BDPars * bd_pars_;
  BDTime last_HB_sent_at_;
  bool had_new_HB_ = false;
  unsigned int words_processed = 0;

  std::unique_ptr<std::vector<EncOutput>> output_block_; // encoder builds up one set of blocks at a time

  BDTime GetRelativeTime() const;
  void RunOnce();
  inline void PushWord(uint32_t word); // helper for Encode, does serialization into output_block_
  inline void PadNopsAndFlush(); // pushes nops until the output_block_ is a multiple of WORDS_PER_BLOCK
  inline void FlushWords(); // flushes words to comm, padding to complete the current block
  void Encode(const std::unique_ptr<std::vector<EncInput>> inputs);

};

}  // bddriver
}  // pystorm

#endif
