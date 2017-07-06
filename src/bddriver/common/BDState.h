#ifndef BDSTATE_H
#define BDSTATE_H

#include <chrono>
#include <string>
#include <vector>
#include <array>

#include "BDPars.h"
#include "DriverPars.h"
#include "common/DriverTypes.h"

namespace pystorm {
namespace bddriver {

/// Keeps track of currently set register values, toggle states, memory values, etc.
///
/// Also encodes timing assumptions: e.g. as soon as the traffic toggles are turned
/// off in software, it is not necessarily safe to start programming memories:
/// there is some amount of time that we must wait for the traffic to drain completely.
/// the length of this delay is kept in DriverPars, and is used by BDState to implement
/// an interface that the driver can use to block until it is safe.
class BDState {
 public:
  BDState(const bdpars::BDPars *bd_pars, const driverpars::DriverPars *driver_pars);
  ~BDState();

  void SetMem(bdpars::MemId mem_id, unsigned int start_addr, const std::vector<BDWord> &data);
  inline const std::vector<BDWord> *GetMem(bdpars::MemId mem_id) const { return &mems_.at(mem_id); }

  void SetReg(bdpars::RegId reg_id, BDWord data);
  const std::pair<const BDWord *, bool> GetReg(bdpars::RegId reg_id) const;

  // A toggle is a special case of register
  void SetToggle(bdpars::RegId reg_id, bool traffic_en, bool dump_en);
  std::tuple<bool, bool, bool> GetToggle(bdpars::RegId reg_id) const;

  bool AreTrafficRegsOff() const;  /// is traffic_en == false for all traffic_regs_
  bool IsTrafficOff() const;       /// has AreTrafficRegsOff been true for traffic_drain_us
  void WaitForTrafficOff() const;  /// Busy wait until IsTrafficOff()

 private:
  // const pointer to driver's pars objects
  const bdpars::BDPars *bd_pars_;
  const driverpars::DriverPars *driver_pars_;

  // register contents
  std::array<BDWord, bdpars::RegIdCount> reg_;
  std::array<bool, bdpars::RegIdCount> reg_valid_;

  // memory contents
  std::array<std::vector<BDWord>, bdpars::MemIdCount> mems_; // conceptually, should be array of arrays, but can't to do that

  // binary vectors denote whether memory entries have been programmed
  std::array<std::vector<bool>, bdpars::MemIdCount> mems_valid_;

  // I iterate through these a lot, this is for convenience
  const std::vector<bdpars::RegId> kTrafficRegs = {
    bdpars::NEURON_DUMP_TOGGLE, 
    bdpars::TOGGLE_PRE_FIFO,
    bdpars::TOGGLE_POST_FIFO0, 
    bdpars::TOGGLE_POST_FIFO1};

  // timing: when certain things happened
  std::chrono::high_resolution_clock::time_point all_traffic_off_start_;

  int TrafficRegWaitTimeLeftUs() const;
};

// equality test, checks all fields
bool operator==(const BDState &lhs, const BDState &rhs);

}  // bddriver
}  // pystorm

#endif
