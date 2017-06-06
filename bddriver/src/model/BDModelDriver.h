#ifndef BDMODELDRIVER_H
#define BDMODELDRIVER_H

#include "Driver.h"
#include "BDModel.h"
#include "comm/CommBDModel.h"

namespace pystorm {
namespace bddriver {

/// Specialization of Driver that uses BDModelComm.
/// I could have made Driver depend on BDModel, and have an optional constructor
/// argument. I opted to subclass instead to contain the dependency to this 
/// particular (testing-only) use case.
class BDModelDriver : public Driver {

 private:
   bdmodel::BDModel * model_;

 public: 
  BDModelDriver() : Driver() {

    model_ = new bdmodel::BDModel(bd_pars_, driver_pars_);

    delete comm_;
    comm_ = new comm::CommBDModel( // overwrite comm_ assignment from base constructor
        model_,
        dec_buf_in_,
        enc_buf_out_);
  }
  ~BDModelDriver() { delete model_; }

  inline bdmodel::BDModel * GetBDModel() { return model_; }
};

}  // bddriver namespace
}  // pystorm namespace

#endif
