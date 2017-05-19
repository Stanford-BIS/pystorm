#ifndef BDMODEL_H
#define BDMODEL_H

#include "common/DriverTypes.h"
#include "common/BDPars.h"
#include "common/BDState.h"
#include "common/DriverPars.h" 

namespace pystorm {
namespace bddriver {
namespace bdmodel {

class BDModel {
public:

  BDModel();
  ~BDModel();

private:

  BDState * state_;

  // needed by state_
  driverpars::DriverPars * driver_pars_;
  bdpars::BDPars * bd_pars_;

};

} // bdmodel
} // bddriver
} // pystorm

#endif
