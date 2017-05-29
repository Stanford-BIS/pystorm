#ifndef BDSTATE_H
#define BDSTATE_H

#include <string>
#include <vector>
#include <chrono>

#include "common/DriverTypes.h"
#include "BDPars.h"
#include "DriverPars.h"

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
    BDState(const bdpars::BDPars * bd_pars, const driverpars::DriverPars * driver_pars);
    ~BDState();

    void SetPAT(unsigned int start_addr, const std::vector<PATData> & data);
    void SetTAT0(unsigned int start_addr, const std::vector<TATData> & data);
    void SetTAT1(unsigned int start_addr, const std::vector<TATData> & data);
    void SetAM(unsigned int start_addr, const std::vector<AMData> & data);
    void SetMM(unsigned int start_addr, const std::vector<MMData> & data);

    inline const std::vector<PATData> * GetPAT() const { return &PAT_; }
    inline const std::vector<TATData> * GetTAT0() const { return &TAT0_; }
    inline const std::vector<TATData> * GetTAT1() const { return &TAT1_; }
    inline const std::vector<AMData> * GetAM() const { return &AM_; }
    inline const std::vector<MMData> * GetMM() const { return &MM_; }

    void SetReg(bdpars::RegId reg_id, const std::vector<unsigned int> & data);
    const std::pair<const std::vector<unsigned int> *, bool> GetReg(bdpars::RegId reg_id) const;

    void SetToggle(bdpars::RegId reg_id, bool traffic_en, bool dump_en);
    std::tuple<bool, bool, bool> GetToggle(bdpars::RegId reg_id) const;

    bool AreTrafficRegsOff() const; /// is traffic_en == false for all traffic_regs_
    bool IsTrafficOff() const; /// has AreTrafficRegsOff been true for traffic_drain_us
    void WaitForTrafficOff() const; /// Busy wait until IsTrafficOff()

  private:

    /// const pointer to driver's pars objects
    const bdpars::BDPars * bd_pars_;
    const driverpars::DriverPars * driver_pars_;

    /// register contents
    std::vector<std::vector<unsigned int> > reg_;
    std::vector<bool> reg_valid_;

    /// memory contents
    std::vector<PATData> PAT_;
    std::vector<TATData> TAT0_;
    std::vector<TATData> TAT1_;
    std::vector<AMData> AM_;
    std::vector<MMData> MM_;

    /// binary vectors denote whether memory entries have been programmed
    std::vector<bool> PAT_valid_;
    std::vector<bool> TAT0_valid_;
    std::vector<bool> TAT1_valid_;
    std::vector<bool> AM_valid_;
    std::vector<bool> MM_valid_;

    /// consts
    const std::vector<bdpars::RegId> kTrafficRegs = {
        bdpars::NEURON_DUMP_TOGGLE, 
        bdpars::TOGGLE_PRE_FIFO, 
        bdpars::TOGGLE_POST_FIFO0, 
        bdpars::TOGGLE_POST_FIFO1
    };

    /// timing: when certain things happened
    std::chrono::high_resolution_clock::time_point all_traffic_off_start_;

    int TrafficRegWaitTimeLeftUs() const; 

};

// equality test, checks all fields
bool operator==(const BDState & lhs, const BDState & rhs);

} // bddriver
} // pystorm

#endif
