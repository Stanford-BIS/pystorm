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

  pars_["DumpPAT"]["timeout_us"] = 1000;
  pars_["DumpTAT"]["timeout_us"] = 1000;
  pars_["DumpMM"]["timeout_us"] = 1000;
  pars_["RecvSpikes"]["timeout_us"] = 1000;
  pars_["RecvTags"]["timeout_us"] = 1000;
}

DriverPars::~DriverPars()
{
}

} // bddriver
} // pystorm
