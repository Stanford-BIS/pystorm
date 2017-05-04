#ifndef DRIVERPARS_H
#define DRIVERPARS_H

#include <string>
#include <unordered_map>

namespace pystorm {
namespace bddriver {

enum DriverParObj {
  // objects
  kenc_buf_in_,
  kenc_buf_out_,
  kdec_buf_in_,
  kdec_buf_out_,
  kenc_,
  kdec_,
  kbd_state_,
  // functions
  kDumpPAT,
  kDumpTAT,
  kDumpMM,
  kRecvSpikes,
  kRecvTags,

  kLastDriverParObj = kRecvTags
};

enum DriverParProperty {
  kcapacity,
  kchunk_size,
  ktraffic_drain_us,
  ktimeout_us,

  kLastDriverParProperty = ktimeout_us
};

class DriverPars {
  public:
    // init from yaml file describing driver parameters
    DriverPars();
    ~DriverPars();

    inline unsigned int Get(DriverParObj object, DriverParProperty property) const 
      { return pars_.at(object).at(property); }

  private:
    std::unordered_map<DriverParObj, std::unordered_map<DriverParProperty, unsigned int> > pars_;

};

} // bddriver
} // pystorm

#endif
