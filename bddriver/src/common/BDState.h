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

class BDState {
  // keeps track of currently set register values, toggle states, memory values, etc.
  public:
    BDState(const BDPars * bd_pars, const DriverPars * driver_pars);
    ~BDState();

    void SetPAT(unsigned int start_addr, const std::vector<PATData> & data);
    void SetTAT0(unsigned int start_addr, const std::vector<TATData> & data);
    void SetTAT1(unsigned int start_addr, const std::vector<TATData> & data);
    void SetAM(unsigned int start_addr, const std::vector<AMData> & data);
    void SetMM(unsigned int start_addr, const std::vector<MMData> & data);

    void SetReg(RegId reg_id, const std::vector<unsigned int> & data);
    const std::pair<const std::vector<unsigned int> *, bool> GetReg(RegId reg_id) const;

    void SetToggle(RegId reg_id, bool traffic_en, bool dump_en);
    std::tuple<bool, bool, bool> GetToggle(RegId reg_id) const;

    bool AreTrafficRegsOff() const; /// is traffic_en == false for all traffic_regs_
    bool IsTrafficOff() const; /// has AreTrafficRegsOff been true for traffic_drain_us
    void WaitForTrafficOff() const;

  private:

    /// const pointer to driver's pars objects
    const BDPars * bd_pars_;
    const DriverPars * driver_pars_;

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
    const std::vector<RegId> kTrafficRegs = {NeuronDumpToggle, TOGGLE_PRE_FIFO, TOGGLE_POST_FIFO0, TOGGLE_POST_FIFO1};

    /// timing: when certain things happened
    std::chrono::high_resolution_clock::time_point all_traffic_off_start_;

};

} // bddriver
} // pystorm

#endif
