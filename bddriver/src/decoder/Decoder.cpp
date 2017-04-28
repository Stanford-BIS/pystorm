#include "Decoder.h"

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <thread>
#include <utility>

#include "common/BDPars.h"
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
  std::vector<unsigned int> num_pushed_to_each(out_bufs_.size(), 0);
  Decode(input_chunk_, num_popped, &output_chunks_, &num_pushed_to_each);

  bool all_success = false;

  // XXX this is not ideal. Blocking on one queue should not cause other queues to block
  
  while (!all_success & do_run_) { // if killed, need to stop trying
    all_success = true;
    for (unsigned int i = 0; i < out_bufs_.size(); i++) {
      if (num_pushed_to_each[i] > 0) {
        bool this_success = out_bufs_[i]->Push(output_chunks_[i], num_pushed_to_each[i], timeout_us_);
        all_success = all_success & this_success;
      }
    }
  }
  //cout << "decoded a chunk" << endl;
  //num_processed_ += num_popped;
}

Decoder::Decoder(
    const BDPars * pars, 
    MutexBuffer<DecInput> * in_buf, 
    const std::vector<MutexBuffer<DecOutput> *> & out_bufs, 
    unsigned int chunk_size, 
    unsigned int timeout_us
) : Xcoder(pars, in_buf, out_bufs, chunk_size, timeout_us) // call default constructor
{
  // set up the vectors we use to do the decoding
  // for now not worrying about the order of entries, might be important later
  for (const auto& it : *(pars_->FunnelRoutes())) {
    uint64_t one = 1;

    uint64_t route_val = static_cast<uint64_t>(it.first);
    uint64_t route_len = static_cast<uint64_t>(it.second);
    uint64_t payload_len = static_cast<uint64_t>(pars_->Width(BD_output)) - route_len;

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

void Decoder::Decode(const DecInput * inputs, unsigned int num_popped, std::vector<DecOutput *> * outputs, std::vector<unsigned int> * num_pushed_to_each)
{
  for (unsigned int i = 0; i < num_popped; i++) {

    // XXX this is where you would decode the FPGA
    // do something with time
    unsigned int time_epoch = 0;

    // XXX this is where you would do something with the core id
    unsigned int core_id = 0;

    // decode funnel
    uint64_t input = inputs[0];
    std::pair<unsigned int, uint32_t> funnel_decoded = DecodeFunnel(input);
    unsigned int leaf_idx = funnel_decoded.first;
    uint32_t payload = funnel_decoded.second;

    //cout << leaf_idx << endl;

    unsigned int num_pushed_to_this = num_pushed_to_each->at(leaf_idx);
    (*outputs)[leaf_idx][num_pushed_to_this] = {payload, core_id, time_epoch};

    (*num_pushed_to_each)[leaf_idx]++;
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
  for (unsigned int i = 0; i < leaf_routes_.size(); i++) {

    // mask out everything except the route bits used by this leaf
    uint64_t eligible_route_bits_only = leaf_route_masks_[i] & payload_route;

    // check if the route bits match
    uint64_t route = leaf_routes_[i];
    
    if (eligible_route_bits_only == route) {

      // mask out payload, make binary
      uint32_t payload = static_cast<uint32_t>(leaf_payload_masks_[i] & payload_route);

      // return payload and leaf name tuple
      unsigned int leaf_idx = i;

      return std::make_pair(leaf_idx, payload);
    }
  }
  assert(false && "input word missed all possible route words: malformed route table");
}

} // bddriver
} // pystorm
