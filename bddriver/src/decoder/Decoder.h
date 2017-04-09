#ifndef DECODER_H
#define DECODER_H

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

typedef std::pair<HWLoc, Binary> DecOutput;
typedef Binary DecInput;

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
    std::pair<std::string, Binary> DecodeFunnel(const Binary& payload_route) const;
    Binary DecodeFPGA(/*TODO args*/ const Binary& input) const;

    std::vector<std::string> leaf_names_;
    std::vector<uint64_t> leaf_routes_;
    std::vector<unsigned int> leaf_route_lens_;
    std::vector<uint64_t> leaf_route_masks_;
    std::vector<uint64_t> leaf_payload_masks_;
    std::vector<unsigned int> leaf_payload_lens_;

};

} // bddriver
} // pystorm

#endif
