#ifndef DRIVERPARS_H
#define DRIVERPARS_H

#include <string>
#include <vector>

namespace pystorm {
namespace bddriver {
namespace driverpars {

enum DriverParId {
  // objects
  ENC_BUF_IN_CAPACITY,
  DEC_BUF_IN_CAPACITY,
  ENC_BUF_OUT_CAPACITY,
  DEC_BUF_OUT_CAPACITY,
  ENC_CHUNK_SIZE,
  ENC_TIMEOUT_US,
  DEC_CHUNK_SIZE,
  DEC_TIMEOUT_US,
  BD_STATE_TRAFFIC_DRAIN_US,
  COMM_TYPE,
  BDMODELCOMM_TRY_FOR_US,
  BDMODELCOMM_MAX_TO_READ,
  BDMODELCOMM_SLEEP_FOR_US,
  // functions
  DUMPPAT_TIMEOUT_US,
  DUMPTAT_TIMEOUT_US,
  DUMPMM_TIMEOUT_US,
  RECVSPIKES_TIMEOUT_US,
  RECVTAGS_TIMEOUT_US,

  LastDriverParId = RECVTAGS_TIMEOUT_US
};

enum DriverStringParId {
  SOFT_COMM_IN_FNAME,
  SOFT_COMM_OUT_FNAME,

  LastDriverStringParId = SOFT_COMM_OUT_FNAME
};

enum CommType {
  SOFT,
  BDMODEL,
  LIBUSB,

  LastCommType = LIBUSB
};

/// Stores parameters that modify driver object parameters/functions
class DriverPars {
 public:
  // init from yaml file describing driver parameters
  DriverPars();
  ~DriverPars();

  inline unsigned int Get(DriverParId par_id) const { return pars_.at(par_id); }
  inline const std::string* Get(DriverStringParId par_id) const { return &(string_pars_.at(par_id)); }

 private:
  std::vector<unsigned int> pars_;
  std::vector<std::string> string_pars_;
};

}  // driverpars
}  // bddriver
}  // pystorm

#endif
