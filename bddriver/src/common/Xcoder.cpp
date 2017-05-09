#include "Xcoder.h"

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#include <thread>

#include "common/BDPars.h"
#include "common/MutexBuffer.h"

#include <iostream>
using std::cout;
using std::endl;

namespace pystorm {
namespace bddriver {

template <class TIN, class TOUT>
Xcoder<TIN, TOUT>::Xcoder(
    const BDPars * pars, 
    MutexBuffer<TIN> * in_buf, 
    const std::vector<MutexBuffer<TOUT> *> & out_bufs, 
    unsigned int input_chunk_size, 
    unsigned int output_chunk_size, 
    unsigned int timeout_us) 
{
  pars_ = pars;  
  in_buf_ = in_buf;
  out_bufs_ = out_bufs;
  timeout_us_ = timeout_us;

  // allocate working arrays
  input_chunk_size_ = input_chunk_size;
  output_chunk_size_ = output_chunk_size; 

  input_chunk_ = new TIN[input_chunk_size_];

  for (unsigned int i = 0; i < out_bufs.size(); i++) {
    output_chunks_.push_back(new TOUT[output_chunk_size_]);
  }
}

template <class TIN, class TOUT>
Xcoder<TIN, TOUT>::~Xcoder()
{
  delete[] input_chunk_;
  for (auto& chunk : output_chunks_) {
    delete[] chunk;
  }
  if (do_run_) {
    Stop();
  }
  delete thread_;
}

template <class TIN, class TOUT>
void Xcoder<TIN, TOUT>::Run()
{
  while(do_run_) {
    RunOnce();
  }
}

template <class TIN, class TOUT>
void Xcoder<TIN, TOUT>::Start()
{
  do_run_ = true;
  thread_ = new std::thread([this]{ this->Run(); });
}

template <class TIN, class TOUT>
void Xcoder<TIN, TOUT>::Stop()
{
  do_run_ = false;
  thread_->join();
}

} // bddriver
} // pystorm
