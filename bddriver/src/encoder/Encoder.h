#ifndef ENCODER_H
#define ENCODER_H

#include <string>
#include <vector>
#include <utility>
#include <thread>

#include "common/BDPars.h"
#include "common/HWLoc.h"
#include "common/Binary.h"
#include "common/MutexBuffer.h"
#include "common/Xcoder.h"

namespace pystorm {
namespace bddriver {

typedef std::pair<HWLoc, Binary> EncInput;
typedef Binary EncOutput;

class Encoder : public Xcoder<EncInput, EncOutput> {
  public:
    Encoder(
        const BDPars * pars, 
        MutexBuffer<EncInput> * in_buf, 
        MutexBuffer<EncOutput> * out_buf, 
        unsigned int chunk_size, 
        unsigned int timeout_us=1000
    ) : Xcoder(pars, in_buf, out_buf, chunk_size, timeout_us) {}; // call base constructor only

  private:

    void RunOnce();
    void Encode(const EncInput * inputs, unsigned int num_popped, EncOutput * outputs);
    Binary EncodeHorn(const Binary& route, const Binary& payload) const; 
    Binary EncodeFPGA(/*TODO args*/ const Binary& payload) const;
    
};

} // bddriver
} // pystorm

#endif
