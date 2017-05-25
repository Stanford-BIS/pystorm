#ifndef BDMODEL_H
#define BDMODEL_H

#include <cstdint>
#include <vector>

#include "common/DriverTypes.h"
#include "common/BDPars.h"
#include "common/BDState.h"
#include "common/DriverPars.h" 
#include "BDModelUtil.h"

namespace pystorm {
namespace bddriver {
namespace bdmodel {

class BDModel {
public:

  BDModel(const bdpars::BDPars * bd_pars, const driverpars::DriverPars * driver_pars);
  ~BDModel();

  void ParseInput(const std::vector<uint8_t> & input_stream);

private:

  // updated by the Process call
  BDState * state_;

  std::vector<SynSpike> received_spikes_;
  std::vector<Tag> received_tags_;

  // intermediate results

  std::vector<MMData> MM_dump_;
  std::vector<AMData> AM_dump_;
  std::vector<PATData> PAT_dump_;
  std::vector<TATData> TAT_dump_[2];

  unsigned int MM_address_ = 0;
  unsigned int AM_address_ = 0;
  unsigned int TAT_address_[2] = {0, 0};

  // needed by state_
  const driverpars::DriverPars * driver_pars_;
  const bdpars::BDPars * bd_pars_;

  void Process(const std::vector<FieldVValues> & inputs);
  void ProcessReg(bdpars::RegId reg_id, const std::vector<FieldVValues> & inputs);
  void ProcessInput(bdpars::InputId input_id, const std::vector<FieldVValues> & inputs);
  void ProcessMM(const std::vector<FieldVValues> & inputs);
  void ProcessAM(const std::vector<FieldVValues> & inputs);
  void ProcessTAT(unsigned int TAT_idx, const std::vector<FieldVValues> & inputs);
  void ProcessPAT(const std::vector<FieldVValues> & inputs);

};

} // bdmodel
} // bddriver
} // pystorm

#endif
