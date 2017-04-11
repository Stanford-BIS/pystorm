#include "Decoder.h"

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#include <thread>
#include <utility>

#include "common/BDPars.h"
#include "common/HWLoc.h"
#include "common/binary_util.h"
#include "common/MutexBuffer.h"

#include <iostream>
using std::cout;
using std::endl;

namespace pystorm {
namespace bddriver {

void Decoder::RunOnce()
{
  unsigned int num_popped = in_buf_->Pop(input_chunk_, max_chunk_size_, timeout_us_);
  Decode(input_chunk_, num_popped, output_chunk_);
  bool success = false;
  while (!success & do_run_) { // if killed, need to stop trying
    success = out_buf_->Push(output_chunk_, num_popped, timeout_us_);
  }
  //num_processed_ += num_popped;
}

Decoder::Decoder(
    const BDPars * pars, 
    MutexBuffer<DecInput> * in_buf, 
    MutexBuffer<DecOutput> * out_buf, 
    unsigned int chunk_size, 
    unsigned int timeout_us
) : Xcoder(pars, in_buf, out_buf, chunk_size, timeout_us) // call default constructor
{
  // set up the vectors we use to do the decoding
  // for now not worrying about the order of entries, might be important later
  for (const auto& it : *(pars_->FunnelRoutes())) {
    leaf_idxs_.push_back(it.first);

    uint64_t one = 1;

    uint64_t route_val = static_cast<uint64_t>(it.first);
    uint64_t route_len = static_cast<uint64_t>(it.second);
    uint64_t payload_len = static_cast<uint64_t>(pars_->Width("BD_output")) - route_len;

    //cout << "route len " << route_len << endl;
    //cout << "payload len " << payload_len << endl;

    leaf_route_lens_.push_back(route_len);
    leaf_payload_lens_.push_back(payload_len);

    // route mask looks like 111100000000
    uint64_t route_mask = ((one << route_len) - one) << payload_len;
    leaf_route_masks_.push_back(route_mask);

    //cout << "route mask " << route_mask << endl;

    // payload mask looks like 000011111111
    uint64_t payload_mask = (one << payload_len) - one;
    leaf_payload_masks_.push_back(payload_mask);

    //cout << "payload mask " << payload_mask << endl;

    // route looks like 101100000000
    //cout << "route " << route_val << endl;
    uint64_t route_shifted = route_val << payload_len;
    leaf_routes_.push_back(route_shifted);

    //cout << "route shifted " << route_shifted << endl;
  }
  //num_processed_ = 0;

}

void Decoder::Decode(const DecInput * inputs, unsigned int num_popped, DecOutput * outputs)
{
  for (unsigned int i = 0; i < num_popped; i++) {

    // XXX this is where you would decode the FPGA

    // XXX this is where you would do something with the core id

    // decode funnel
    uint64_t input = inputs[0];
    std::pair<unsigned int, uint32_t> funnel_decoded = DecodeFunnel(input);
    unsigned int leaf_idx = funnel_decoded.first;
    uint32_t payload = funnel_decoded.second;

    HWLoc loc;
    loc.leaf_idx_ = leaf_idx;
    loc.core_id_ = 0;

    outputs[i] = std::make_pair(loc, payload);
  }
}

inline std::pair<unsigned int, uint32_t> Decoder::DecodeFunnel(uint64_t payload_route) const
{
  // XXX this could probably be optimized to be less branch-y
  // this could also be optimized if the FPGA was used to decode the variable-width
  // route into a fixed-width route, then it would just be a lookup
  
  // msb <- lsb
  // [ route | X | payload ]

  // test which routes match, break out payload for the one that does, return matching leaf name
  for (unsigned int i = 0; i < leaf_idxs_.size(); i++) {

    // mask out everything except the route bits used by this leaf
    uint64_t eligible_route_bits_only = leaf_route_masks_[i] & payload_route;

    // check if the route bits match
    uint64_t route = leaf_routes_[i];
    
    if (eligible_route_bits_only == route) {

      // mask out payload, make binary
      uint32_t payload = static_cast<uint32_t>(leaf_payload_masks_[i] & payload_route);

      // return payload and leaf name tuple
      unsigned int leaf_idx = leaf_idxs_[i];

      return std::make_pair(leaf_idx, payload);
    }
  }
  assert(false && "input word missed all possible route words: malformed route table");
}

} // bddriver
} // pystorm
