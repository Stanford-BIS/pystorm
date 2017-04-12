#include "DriverPars.h"

namespace pystorm {
namespace bddriver {

DriverPars::DriverPars()
{
  pars_["enc_buf_in_"]["capacity"] = 10000;
  pars_["enc_buf_out_"]["capacity"] = 10000;
  pars_["dec_buf_in_"]["capacity"] = 10000;
  pars_["dec_buf_out_"]["capacity"] = 10000;

  pars_["enc_"]["chunk_size"] = 1000;
  pars_["enc_"]["timeout_us"] = 1000;

  pars_["dec_"]["chunk_size"] = 1000;
  pars_["dec_"]["timeout_us"] = 1000;
}

DriverPars::~DriverPars()
{
}

} // bddriver
} // pystorm
