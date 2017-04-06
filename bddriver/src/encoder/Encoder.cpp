#include "Encoder.h"

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#include <thread>

#include "common/BDPars.h"
#include "common/HWLoc.h"
#include "common/Binary.h"
#include "common/MutexBuffer.h"

#include <iostream>
using std::cout;
using std::endl;

namespace pystorm {
namespace bddriver {

void Encoder::RunOnce()
{
  unsigned int num_popped = in_buf_->Pop(input_chunk_, max_chunk_size_, timeout_us_);
  Encode(input_chunk_, num_popped, output_chunk_);
  out_buf_->Push(output_chunk_, num_popped, timeout_us_);
}

void Encoder::Encode(const EncInput * inputs, unsigned int num_popped, EncOutput * outputs)
{
  for (unsigned int i = 0; i < num_popped; i++) {

    // unpack data
    HWLoc destination = inputs[i].first;
    Binary payload = inputs[i].second;

    // look up route for this leaf_name
    const Binary * leaf_route = pars_->LeafRoute(*destination.LeafName());

    // XXX this is where you would do something with the chip id

    // encoder horn
    Binary horn_encoded = EncodeHorn(*leaf_route, payload);

    // XXX this is where you would encode the FPGA
    
    outputs[i] =  horn_encoded;
  }
}

Binary Encoder::EncodeHorn(const Binary& route, const Binary& payload) const
{
  // msb <- lsb
  // [ X | payload | route ]

  //const std::vector<Binary> to_concat = {route, payload};
  Binary encoded = Binary({route, payload});
  //cout << "encoding: " << payload.AsString() << " & " << route.AsString() << " = " << encoded.AsUint() << endl;
  return encoded;
}


Binary Encoder::EncodeFPGA(/*TODO args*/ const Binary& payload) const
{
  // TODO
  return payload;
}

} // bddriver
} // pystorm
