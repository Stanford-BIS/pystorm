#ifndef DRIVERPARS_H
#define DRIVERPARS_H

#include <string>
#include <vector>

namespace pystorm {
namespace bddriver {

enum DriverParId {
  // objects
  enc_buf_in_capacity,
  dec_buf_in_capacity,
  enc_buf_out_capacity,
  dec_buf_out_capacity,
  enc_chunk_size,
  enc_timeout_us,
  dec_chunk_size,
  dec_timeout_us,
  bd_state_traffic_drain_us,
  // functions
  DumpPAT_timeout_us,
  DumpTAT_timeout_us,
  DumpMM_timeout_us,
  RecvSpikes_timeout_us,
  RecvTags_timeout_us,

  LastDriverParId = RecvTags_timeout_us
};

class DriverPars {
  public:
    // init from yaml file describing driver parameters
    DriverPars();
    ~DriverPars();

    inline unsigned int Get(DriverParId par_id) const 
      { return pars_.at(par_id); }

  private:
    std::vector<unsigned int> pars_;

};

} // bddriver
} // pystorm

#endif