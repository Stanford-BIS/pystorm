#include "DriverPars.h"

namespace pystorm {
namespace bddriver {

DriverPars::DriverPars()
{
  const unsigned int ms = 1000;

  // buffer capacities
  pars_[kenc_buf_in_][kcapacity] = 10000;
  pars_[kenc_buf_out_][kcapacity] = 10000;
  pars_[kdec_buf_in_][kcapacity] = 10000;
  pars_[kdec_buf_out_][kcapacity] = 10000;

  // encoder/decoder working chunk sizes
  pars_[kenc_][kchunk_size] = 1 * ms;
  pars_[kenc_][ktimeout_us] = 1 * ms;

  pars_[kdec_][kchunk_size] = 1 * ms;
  pars_[kdec_][ktimeout_us] = 1 * ms;

  pars_[kbd_state_][ktraffic_drain_us] = 1 * ms; // timing assumption: this long after shutting off traffic, bd will be inactive

  // timeouts for functions that pop from buffers
  pars_[kDumpPAT][ktimeout_us] = 1 * ms;
  pars_[kDumpTAT][ktimeout_us] = 1 * ms;
  pars_[kDumpMM][ktimeout_us] = 1 * ms;
  pars_[kRecvSpikes][ktimeout_us] = 1 * ms;
  pars_[kRecvTags][ktimeout_us] = 1 * ms;
}

DriverPars::~DriverPars()
{
}

} // bddriver
} // pystorm
