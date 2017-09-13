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
    uint32_t input_bytes32[bytesPerInput];
    for (unsigned int j = 0; j < bytesPerInput; j++) {
      input_bytes32[j] = static_cast<uint32_t>(inputs[i * bytesPerInput + j]);
    }
    unsigned int byte_widths[bytesPerInput];
    for (unsigned int j = 0; j < bytesPerInput; j++) {
      byte_widths[j] = 8;
    }

    uint32_t input = Pack32(input_bytes32, byte_widths, bytesPerInput);

    // XXX this is where you would decode the FPGA
    // do something with time
    unsigned int time_epoch = 0;

    // XXX this is where you would do something with the core id
    unsigned int core_id = 0;

    // decode funnel
    uint8_t leaf_idx;
    uint32_t payload;

    std::tie(leaf_idx, payload) = DecodeFunnel(input);

    unsigned int num_pushed_to_this          = num_pushed_to_each->at(leaf_idx);
    (*outputs)[leaf_idx][num_pushed_to_this] = {payload, core_id, time_epoch};

    (*num_pushed_to_each)[leaf_idx]++;
  }
}

inline std::pair<uint8_t, uint32_t> Decoder::DecodeFunnel(uint32_t input) const {
  uint8_t leaf_idx;
  uint32_t payload;

  // FPGA-BD word is
  // | leaf ID (8 bits) | Payload (24 bits) |

  leaf_idx = (input >> 24) & 0xFF;
  payload = input & 0x00FFFFFF;
  return std::make_pair(leaf_idx, payload);
}

}  // bddriver
}  // pystorm
