#ifndef ENCODER_H
#define ENCODER_H

#include <string>
#include <vector>
#include <utility>
#include <unordered_map>

#include "bddriver/common/BDPars.h"
#include "bddriver/common/HWLoc.h"
#include "bddriver/common/Binary.h"
#include "bddriver/common/MutexBuffer.h"

namespace pystorm {
namespace bddriver {

typedef std::pair<HWLoc, Binary> EncInput;
typedef Binary EncOutput;

class Encoder {
  public:
    Encoder(const BDPars * pars, MutexBuffer<EncInput> * in, MutexBuffer<EncOutput> * out, unsigned int chunk_size);

    void RunOnce();

  private:
    const BDPars * pars_; // chip parameters, contains routing table used in EncodeHorn

    MutexBuffer<EncInput> * in_buf_; // input buffer
    MutexBuffer<EncOutput> * out_buf_; // output buffer

    unsigned int max_chunk_size_; // max chunk size of inputs processed
    EncInput * input_chunk_; // will point to scratch pad memory for inputs
    EncOutput * output_chunk_; // and outputs

    void Encode(const EncInput * inputs, unsigned int num_popped, EncOutput * outputs) const;
    Binary EncodeHorn(const Binary& route, const Binary& payload) const; 
    Binary EncodeFPGA(/*TODO args*/ const Binary& payload) const;
    
};

} // bddriver
} // pystorm

#endif
