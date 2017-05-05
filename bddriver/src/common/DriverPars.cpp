#include "DriverPars.h"

namespace pystorm {
namespace bddriver {

DriverPars::DriverPars()
{
  const unsigned int ms = 1000;

  // buffer capacities
  pars_.resize(LastDriverParId);
  pars_[enc_buf_in_capacity] = 10000;
  pars_[enc_buf_out_capacity] = 10000;
  pars_[dec_buf_in_capacity] = 10000;
  pars_[dec_buf_out_capacity] = 10000;

  // encoder/decoder working chunk sizes
  pars_[enc_chunk_size] = 1 * ms;
  pars_[enc_timeout_us] = 1 * ms;

  pars_[dec_chunk_size] = 1 * ms;
  pars_[dec_timeout_us] = 1 * ms;

  pars_[bd_state_traffic_drain_us] = 1 * ms; // timing assumption: this long after shutting off traffic, bd will be inactive

  // timeouts for functions that pop from buffers
  pars_[DumpPAT_timeout_us] = 1 * ms;
  pars_[DumpTAT_timeout_us] = 1 * ms;
  pars_[DumpMM_timeout_us] = 1 * ms;
  pars_[RecvSpikes_timeout_us] = 1 * ms;
  pars_[RecvTags_timeout_us] = 1 * ms;
}

DriverPars::~DriverPars()
{
}

} // bddriver
} // pystorm
