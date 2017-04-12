#include <iostream>

#include "Driver.h"

#include "common/BDPars.h"
#include "common/DriverPars.h"
#include "common/HWLoc.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include "common/MutexBuffer.h"

namespace pystorm
{
namespace bddriver
{

Driver& Driver::getInstance()
{
    // In C++11, if control from two threads occurs concurrently, execution
    // shall wait during static variable initialization, therefore, this is 
    // thread safe
    static Driver m_instance;
    return m_instance;
}


Driver::Driver()
{
  // load parameters
  driver_pars_ = new DriverPars();
  bd_pars_ = new BDPars();
  
  // initialize buffers
  enc_buf_in_ = new MutexBuffer<EncInput>(driver_pars_->Get("enc_buf_in_", "capacity"));
  enc_buf_out_ = new MutexBuffer<EncOutput>(driver_pars_->Get("enc_buf_out_", "capacity"));
  dec_buf_in_ = new MutexBuffer<DecInput>(driver_pars_->Get("dec_buf_in_", "capacity"));
  dec_buf_out_ = new MutexBuffer<DecOutput>(driver_pars_->Get("dec_buf_out_", "capacity"));

  // initialize Encoder and Decoder
  enc_ = new Encoder(
      bd_pars_,
      enc_buf_in_, 
      enc_buf_out_, 
      driver_pars_->Get("enc_", "chunk_size"), 
      driver_pars_->Get("enc_", "timeout_us")
  );

  dec_ = new Decoder(
      bd_pars_, 
      dec_buf_in_, 
      dec_buf_out_, 
      driver_pars_->Get("dec_", "chunk_size"), 
      driver_pars_->Get("dec_", "timeout_us")
  );
}

Driver::~Driver()
{
  delete driver_pars_;
  delete bd_pars_;
  delete enc_buf_in_;
  delete enc_buf_out_;
  delete dec_buf_in_;
  delete dec_buf_out_;
  delete enc_;
  delete dec_;
}

void Driver::testcall(const std::string& msg)
{
    std::cout << msg << std::endl;
}

void Driver::Start()
{
  // start all worker threads
  enc_->Start();
  dec_->Start();
}


} // bddriver namespace
} // pystorm namespace
