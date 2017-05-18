#ifndef DECODER_H
#define DECODER_H

#include <cstdint>
#include <string>
#include <vector>
#include <utility>
#include <thread>

#include "common/BDPars.h"
#include "common/MutexBuffer.h"
#include "common/Xcoder.h"
#include "common/DriverTypes.h"

namespace pystorm {
namespace bddriver {

class Decoder : public Xcoder<DecInput, DecOutput> {

  public:

    const static unsigned int bytesPerInput = 5; // BD_output is 34 bits, fits in 5 bytes

    Decoder(
        const bdpars::BDPars * pars, 
        MutexBuffer<DecInput> * in_buf, 
        const std::vector<MutexBuffer<DecOutput> *> & out_bufs, 
        unsigned int chunk_size, 
        unsigned int timeout_us=1000
    );
    ~Decoder();

    // for testing
    //unsigned int num_processed_;

  private:

    void RunOnce();
    std::pair<unsigned int, uint32_t> DecodeFunnel(uint64_t payload_route) const;
    void Decode(const DecInput * inputs, unsigned int num_popped, std::vector<DecOutput *> * outputs, std::vector<unsigned int> * num_pushed_to_each) const;

    std::vector<uint64_t> leaf_routes_;
    std::vector<unsigned int> leaf_route_lens_;
    std::vector<uint64_t> leaf_route_masks_;
    std::vector<uint64_t> leaf_payload_masks_;
    std::vector<unsigned int> leaf_payload_lens_;

};

} // bddriver
} // pystorm

#endif
