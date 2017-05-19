#include "BDModel.h"

namespace pystorm {
namespace bddriver {
namespace bdmodel {

BDModel::BDModel() 
{
  driver_pars_ = new driverpars::DriverPars();
  bd_pars_ = new bdpars::BDPars();

  state_ = new BDState(bd_pars_, driver_pars_);
}

BDModel::~BDModel()
{
  delete state_;
  delete driver_pars_;
  delete bd_pars_;
}

} // bdmodel
} // bddriver
} // pystorm
