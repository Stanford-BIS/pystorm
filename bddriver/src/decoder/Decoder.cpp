#include "Decoder.h"

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#include <thread>
#include <utility>

#include "common/BDPars.h"
#include "common/HWLoc.h"
#include "common/Binary.h"
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
    leaf_names_.push_back(it.first);

    uint64_t one = 1;

    uint64_t route_len = it.second.Bitwidth();
    uint64_t payload_len = pars_->Width("BD_output") - route_len;

    leaf_route_lens_.push_back(route_len);
    leaf_payload_lens_.push_back(payload_len);

    // route mask looks like 111100000000
    uint64_t route_mask = ((one << route_len) - one) << payload_len;
    leaf_route_masks_.push_back(route_mask);

    // payload mask looks like 000011111111
    uint64_t payload_mask = (one << payload_len) - one;
    leaf_payload_masks_.push_back(payload_mask);

    // route looks like 101100000000
    uint64_t route_shifted = it.second.AsUint() << payload_len;
    leaf_routes_.push_back(route_shifted);
  }
  //num_processed_ = 0;

}

void Decoder::Decode(const DecInput * inputs, unsigned int num_popped, DecOutput * outputs)
{
  for (unsigned int i = 0; i < num_popped; i++) {

    // XXX this is where you would decode the FPGA
    Binary FPGA_decoded = inputs[i];
    unsigned int chip_id = 0;

    // XXX this is where you would do something with the chip id
    Binary chip_decoded = FPGA_decoded;

    // decode funnel
    std::pair<std::string, Binary> funnel_decoded = DecodeFunnel(chip_decoded);
    std::string leaf_name = funnel_decoded.first;
    Binary payload = funnel_decoded.second;

    HWLoc loc(chip_id, leaf_name);

    outputs[i] = std::make_pair(loc, payload);
  }
}

std::pair<std::string, Binary> Decoder::DecodeFunnel(const Binary& payload_route) const
{
  // msb <- lsb
  // [ route | X | payload ]

  uint64_t payload_route_val = payload_route.AsUint();

  // test which routes match, break out payload for the one that does, return matching leaf name
  for (unsigned int i = 0; i < leaf_names_.size(); i++) {

    // mask out everything except the route bits used by this leaf
    uint64_t eligible_route_bits_only = leaf_route_masks_[i] & payload_route_val;

    // check if the route bits match
    uint64_t route = leaf_routes_[i];
    if (eligible_route_bits_only == route) {

      // mask out payload, make binary
      uint64_t payload_val = leaf_payload_masks_[i] & payload_route_val;
      uint64_t payload_len = leaf_payload_lens_[i];
      Binary payload(payload_val, payload_len);

      // return payload and leaf name tuple
      std::string leaf = leaf_names_[i];

      return std::make_pair(leaf, payload);
    }
  }
  assert(false && "input word missed all possible route words: malformed route table");
}


Binary Decoder::DecodeFPGA(/*TODO args*/ const Binary& payload) const
{
  // TODO
  return payload;
}

} // bddriver
} // pystorm
