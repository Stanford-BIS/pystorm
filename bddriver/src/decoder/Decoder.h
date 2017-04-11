#ifndef DECODER_H
#define DECODER_H

#include <cstdint>
#include <string>
#include <vector>
#include <utility>
#include <thread>

#include "common/BDPars.h"
#include "common/HWLoc.h"
#include "common/MutexBuffer.h"
#include "common/Xcoder.h"

namespace pystorm {
namespace bddriver {

typedef std::pair<HWLoc, uint32_t> DecOutput;
typedef uint64_t DecInput;

class Decoder : public Xcoder<DecInput, DecOutput> {
  public:
    Decoder(
        const BDPars * pars, 
        MutexBuffer<DecInput> * in_buf, 
        MutexBuffer<DecOutput> * out_buf, 
        unsigned int chunk_size, 
        unsigned int timeout_us=1000
    );

    // for testing
    //unsigned int num_processed_;
    void Decode(const DecInput * inputs, unsigned int num_popped, DecOutput * outputs);

  private:

    void RunOnce();
    std::pair<unsigned int, uint32_t> DecodeFunnel(uint64_t payload_route) const;

    std::vector<unsigned int> leaf_idxs_;
    std::vector<uint64_t> leaf_routes_;
    std::vector<unsigned int> leaf_route_lens_;
    std::vector<uint64_t> leaf_route_masks_;
    std::vector<uint64_t> leaf_payload_masks_;
    std::vector<unsigned int> leaf_payload_lens_;

};

} // bddriver
} // pystorm

#endif
