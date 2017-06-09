#include "DriverPars.h"

namespace pystorm {
namespace bddriver {
namespace driverpars {

DriverPars::DriverPars() {
  const unsigned int ms = 1000;

  pars_.resize(LastDriverParId + 1);
  string_pars_.resize(LastDriverStringParId + 1);

  // Comm type
  pars_[COMM_TYPE] = BDMODEL;

  // CommSoft parameters
  string_pars_[SOFT_COMM_IN_FNAME]  = "soft_comm_in.dat";
  string_pars_[SOFT_COMM_OUT_FNAME] = "soft_comm_out.dat";

  // CommBDModel parameters (XXX some of these might be more general--CommSoft should have DriverPars too)
  pars_[BDMODELCOMM_TRY_FOR_US] = 1000;
  pars_[BDMODELCOMM_MAX_TO_READ] = 10000;
  pars_[BDMODELCOMM_SLEEP_FOR_US] = 10 * ms;

  // buffer capacities
  pars_[ENC_BUF_IN_CAPACITY]  = 10000;
  pars_[ENC_BUF_OUT_CAPACITY] = 10000;
  pars_[DEC_BUF_IN_CAPACITY]  = 64*1024;
  pars_[DEC_BUF_OUT_CAPACITY] = 64*1024;

  // encoder/decoder working chunk sizes
  pars_[ENC_CHUNK_SIZE] = 1 * ms;
  pars_[ENC_TIMEOUT_US] = 1 * ms;

  pars_[DEC_CHUNK_SIZE] = 1 * ms;
  pars_[DEC_TIMEOUT_US] = 1 * ms;

  pars_[BD_STATE_TRAFFIC_DRAIN_US] =
      1 * ms;  // timing assumption: this long after shutting off traffic, bd will be inactive

  // timeouts for functions that pop from buffers
  pars_[DUMPPAT_TIMEOUT_US]    = 1 * ms;
  pars_[DUMPTAT_TIMEOUT_US]    = 1 * ms;
  pars_[DUMPMM_TIMEOUT_US]     = 1 * ms;
  pars_[RECVSPIKES_TIMEOUT_US] = 1 * ms;
  pars_[RECVTAGS_TIMEOUT_US]   = 1 * ms;
}

DriverPars::~DriverPars() {}

}  // driverpars
}  // bddriver
}  // pystorm
