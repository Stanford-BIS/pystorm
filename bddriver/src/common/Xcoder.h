#ifndef XCODER_H
#define XCODER_H

#include <atomic>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "common/BDPars.h"
#include "common/MutexBuffer.h"

namespace pystorm {
namespace bddriver {

template <class TIN, class TOUT>
class Xcoder {
  // pure-virtual base class for encoders/decoders. Derived classes must define RunOnce()
 public:
  Xcoder(const bdpars::BDPars *pars, MutexBuffer<TIN> *in_buf, const std::vector<MutexBuffer<TOUT> *> &out_bufs,
         unsigned int input_chunk_size, unsigned int output_chunk_size, unsigned int timeout_us);
  virtual ~Xcoder();

  void Start();
  void Stop();

 protected:
  const bdpars::BDPars *pars_;  // chip parameters, contains routing table used in EncodeHorn

  MutexBuffer<TIN> *in_buf_;                   // input buffer
  std::vector<MutexBuffer<TOUT> *> out_bufs_;  // output buffer

  unsigned int timeout_us_;            // max condition variable wait time
  unsigned int input_chunk_size_;      // max chunk size of inputs processed
  unsigned int output_chunk_size_;     // max chunk size of outputs processed
  TIN *input_chunk_;                   // will point to scratch pad memory for inputs
  std::vector<TOUT *> output_chunks_;  // and outputs

  std::thread *thread_;       // pointer to thread which will be launched with Start()
  std::atomic<bool> do_run_;  // used to join thread on destruction

  void Run();
  virtual void RunOnce() = 0;
};

}  // bddriver
}  // pystorm

// we have defined a template class. We must include the implementation
#include "Xcoder.cpp"

#endif
