#ifndef ENCODER_H
#define ENCODER_H

#include <cstdint>
#include <vector>
#include <utility>
#include <thread>

#include "common/BDPars.h"
#include "common/MutexBuffer.h"
#include "common/Xcoder.h"
#include "common/DriverTypes.h"

namespace pystorm {
namespace bddriver {

class Encoder : public Xcoder<EncInput, EncOutput> {

  public:

    const static unsigned int bytesPerOutput = 3; // BD_input is 21 bits, fits in 3 bytes


    Encoder(
        const bdpars::BDPars * pars, 
        MutexBuffer<EncInput> * in_buf, 
        MutexBuffer<EncOutput> * out_buf, 
        unsigned int chunk_size, 
        unsigned int timeout_us=1000
    ) : Xcoder(pars, in_buf, {out_buf}, chunk_size, chunk_size * bytesPerOutput, timeout_us) {}; // call base constructor only
    ~Encoder() {};

  private:

    void RunOnce();
    void Encode(const EncInput * inputs, unsigned int num_popped, EncOutput * outputs) const;
    uint32_t EncodeHorn(bdpars::FHRoute route, uint32_t payload) const;
    
};

} // bddriver
} // pystorm

#endif
