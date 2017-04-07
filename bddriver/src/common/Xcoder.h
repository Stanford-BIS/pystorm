#ifndef XCODER_H
#define XCODER_H

#include <string>
#include <vector>
#include <utility>
#include <thread>

#include "common/BDPars.h"
#include "common/MutexBuffer.h"

namespace pystorm {
namespace bddriver {

template <class TIN, class TOUT>
class Xcoder {
  // pure-virtual base class for encoders/decoders. Derived classes must define RunOnce()
  public: 
    Xcoder(
        const BDPars * pars, 
        MutexBuffer<TIN> * in_buf, 
        MutexBuffer<TOUT> * out_buf, 
        unsigned int chunk_size, 
        unsigned int timeout_us
    );
    ~Xcoder();

    void Start();
    void Stop();

  protected:
    const BDPars * pars_; // chip parameters, contains routing table used in EncodeHorn

    MutexBuffer<TIN> * in_buf_; // input buffer
    MutexBuffer<TOUT> * out_buf_; // output buffer

    unsigned int timeout_us_; // max condition variable wait time
    unsigned int max_chunk_size_; // max chunk size of inputs processed
    TIN * input_chunk_; // will point to scratch pad memory for inputs
    TOUT * output_chunk_; // and outputs

    std::thread * thread_; // pointer to thread which will be launched with Start()
    bool do_run_; // used to join thread on destruction

    void Run();
    virtual void RunOnce() = 0;
};

} // bddriver
} // pystorm

// we have defined a template class. We must include the implementation
#include "Xcoder.cpp"

#endif