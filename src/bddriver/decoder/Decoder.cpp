#include "Decoder.h"

#include <cstdint>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "common/BDPars.h"
#include "common/MutexBuffer.h"
#include "common/binary_util.h"

#include <iostream>
using std::cout;
using std::endl;

namespace pystorm {
namespace bddriver {

void Decoder::RunOnce() {
  unsigned int num_popped = in_buf_->Pop(input_chunk_, input_chunk_size_, timeout_us_, bytesPerInput);
  std::vector<unsigned int> num_pushed_to_each(out_bufs_.size(), 0);
  Decode(input_chunk_, num_popped, &output_chunks_, &num_pushed_to_each);

  unsigned int total_pushed_to_each = 0;
  for (auto& it : num_pushed_to_each) {
    total_pushed_to_each += it;
  }

  // XXX can use the Read/PopAfterRead ifc, but doesn't seem to improve throughput
  // const DecInput * read_data;
  // unsigned int num_read;
  // std::tie(read_data, num_read) = in_buf_->Read(input_chunk_size_, timeout_us_);
  // std::vector<unsigned int> num_pushed_to_each(out_bufs_.size(), 0);
  // Decode(read_data, num_read, &output_chunks_, &num_pushed_to_each);
  // in_buf_->PopAfterRead();

  // XXX this is not ideal. Blocking on one queue should not cause other queues to block
  // hard to get around due to the serial nature of the input, however
  // in practice, shouldn't be a big issue

  for (unsigned int i = 0; i < out_bufs_.size(); i++) {
    if (num_pushed_to_each[i] > 0) {
      bool success = false;
      // have to check do_run_, since we could potentially get stuck here
      while (!success && do_run_) {
        success = out_bufs_[i]->Push(output_chunks_[i], num_pushed_to_each[i], timeout_us_);
      }
    }
  }
  // num_processed_ += num_popped;
  // cout << "decoded a chunk size " << num_popped << ". total " << num_processed_ << endl;
}

Decoder::Decoder(
    const bdpars::BDPars* pars,
    MutexBuffer<DecInput>* in_buf,
    const std::vector<MutexBuffer<DecOutput>*>& out_bufs,
    unsigned int chunk_size,
    unsigned int timeout_us)
    : Xcoder(pars, in_buf, out_bufs, chunk_size * bytesPerInput, chunk_size, timeout_us)  // call default constructor
{
  // set up the vectors we use to do the decoding

  for (const bdpars::FHRoute& it : *(pars_->FunnelRoutes())) {
    uint64_t one = 1;

    uint64_t route_val   = static_cast<uint64_t>(it.first);
    uint64_t route_len   = static_cast<uint64_t>(it.second);
    uint64_t payload_len = static_cast<uint64_t>(pars_->Width(bdpars::BD_OUTPUT)) - route_len;

    // cout << "route len " << route_len << endl;
    // cout << "payload len " << payload_len << endl;

    leaf_route_lens_.push_back(route_len);
    leaf_payload_lens_.push_back(payload_len);

    // route mask looks like 111100000000
    uint64_t route_mask = ((one << route_len) - one) << payload_len;
    leaf_route_masks_.push_back(route_mask);

    // cout << "route mask " << route_mask << endl;

    // payload mask looks like 000011111111
    uint64_t payload_mask = (one << payload_len) - one;
    leaf_payload_masks_.push_back(payload_mask);

    // cout << "payload mask " << payload_mask << endl;

    // route looks like 101100000000
    // cout << "route " << route_val << endl;
    uint64_t route_shifted = route_val << payload_len;
    leaf_routes_.push_back(route_shifted);

    // cout << "route shifted " << route_shifted << endl;
  }

  // payload shifts are all zero for decoder
  leaf_payload_shifts_ = std::vector<unsigned int>(leaf_routes_.size(), 0);

  // num_processed_ = 0;
}

Decoder::~Decoder() {
  // cout << "decoder processed " << num_processed_ << " entries" << endl;
}

void Decoder::Decode(
    const DecInput* inputs,
    unsigned int num_popped,
    std::vector<DecOutput*>* outputs,
    std::vector<unsigned int>* num_pushed_to_each) const {
  assert(num_popped % bytesPerInput == 0 && "should have used Pop() with a multiple argument");

  for (unsigned int i = 0; i < num_popped / bytesPerInput; i++) {
    // Pack 5 bytes into uint64_t
    // this is the safe way, there's probably a lower-level, sketchier way that relies on endianess
    // XXX this should probably be optimized: cuts decoder throughput in half vs taking in uint64_t
    uint64_t input_bytes64[bytesPerInput];
    for (unsigned int j = 0; j < bytesPerInput; j++) {
      input_bytes64[j] = static_cast<uint64_t>(inputs[i * bytesPerInput + j]);
    }
    unsigned int byte_widths[bytesPerInput];
    for (unsigned int j = 0; j < bytesPerInput; j++) {
      byte_widths[j] = 8;
    }

    uint64_t input = Pack64(input_bytes64, byte_widths, bytesPerInput);

    // XXX this is where you would decode the FPGA
    // do something with time
    unsigned int time_epoch = 0;

    // XXX this is where you would do something with the core id
    unsigned int core_id = 0;

    // decode funnel
    unsigned int leaf_idx;
    uint32_t payload;
    std::tie(leaf_idx, payload) =
        DecodeFH<uint64_t, uint32_t>(input, leaf_routes_, leaf_route_masks_, leaf_payload_masks_, leaf_payload_shifts_);

    // std::tie(leaf_idx, payload) = DecodeFunnel(input);

    unsigned int num_pushed_to_this          = num_pushed_to_each->at(leaf_idx);
    (*outputs)[leaf_idx][num_pushed_to_this] = {payload, core_id, time_epoch};

    (*num_pushed_to_each)[leaf_idx]++;
  }
}

inline std::pair<unsigned int, uint32_t> Decoder::DecodeFunnel(uint64_t payload_route) const {
  assert(false && "deprecated");
  // XXX this could probably be optimized to be less branch-y
  // this could also be optimized if the FPGA was used to decode the variable-width
  // route into a fixed-width route, then it would just be a lookup

  // msb <- lsb
  // [ route | X | payload ]

  // test which routes match, break out payload for the one that does, return matching leaf name
  // cout << "payload_route" << endl;
  // cout << UintAsString(payload_route, 34) << endl;
  for (unsigned int i = 0; i < leaf_routes_.size(); i++) {
    // cout << "trying leaf " << i << endl;

    // mask out everything except the route bits used by this leaf
    uint64_t eligible_route_bits_only = leaf_route_masks_[i] & payload_route;

    // cout << "eligible bits: ";
    // cout << UintAsString(eligible_route_bits_only, 34) << " = " << eligible_route_bits_only << endl;

    // check if the route bits match
    uint64_t route = leaf_routes_[i];
    // cout << "route: " << route << endl;

    if (eligible_route_bits_only == route) {
      // mask out payload, make binary
      uint32_t payload = static_cast<uint32_t>(leaf_payload_masks_[i] & payload_route);

      // return payload and leaf name tuple
      unsigned int leaf_idx = i;
      // cout << "HIT" << endl;

      return std::make_pair(leaf_idx, payload);
    }
  }
  assert(false && "input word missed all possible route words: malformed route table");
  return std::make_pair(0, 0);  // squelch compiler warning
}

}  // bddriver
}  // pystorm
