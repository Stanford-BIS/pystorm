#include "BDDriver/Encoder/Encoder.h"

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>

#include "BDDriver/common/BDPars.h"
#include "BDDriver/common/HWLoc.h"
#include "BDDriver/common/Binary.h"
#include "BDDriver/common/MutexBuffer.h"

namespace pystorm {
namespace bddriver {

Encoder::Encoder(const BDPars * pars, MutexBuffer<EncInput> * in_buf, MutexBuffer<EncOutput> * out_buf, unsigned int chunk_size) 
{
  pars_ = pars;  
  in_buf_ = in_buf;
  out_buf_ = out_buf;

  // allocate s
  max_chunk_size_ = chunk_size;
  input_chunk_ = new EncInput[max_chunk_size_];
  output_chunk_ = new EncOutput[max_chunk_size_];
}

void Encoder::RunOnce()
{
  unsigned int num_popped = in_buf_->Pop(input_chunk_, max_chunk_size_);
  Encode(input_chunk_, num_popped, output_chunk_);
  out_buf_->Push(output_chunk_, num_popped);
}

void Encoder::Encode(const EncInput * inputs, unsigned int num_popped, EncOutput * outputs) const
{
  for (unsigned int i = 0; i < num_popped; i++) {

    // unpack data
    HWLoc destination = inputs[i].first;
    Binary payload = inputs[i].second;

    // look up route for this leaf_name
    Binary * leaf_route = pars_->LeafRoute(*destination.LeafName());

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

  const std::vector<Binary> to_concat = {route, payload};
  return Binary(to_concat);
}


Binary Encoder::EncodeFPGA(/*TODO args*/ const Binary& payload) const
{
  // TODO
  return payload;
}

} // bddriver
} // pystorm
