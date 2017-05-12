#include "BDState.h"

#include <vector>
#include <assert.h>

#include "BDPars.h"

namespace pystorm {
namespace bddriver {

#include <iostream>
using std::cout;
using std::endl;

BDState::BDState(const BDPars * bd_pars, const DriverPars * driver_pars)
{

  bd_pars_ = bd_pars;
  driver_pars_ = driver_pars;

  // initialize memory vectors
  PAT_  = std::vector<PATData>(bd_pars->Size(PAT), {0,0,0});
  TAT0_ = std::vector<TATData>(bd_pars->Size(TAT0), {0,0,0,0,0,0,0,0,0,0});
  TAT1_ = std::vector<TATData>(bd_pars->Size(TAT1), {0,0,0,0,0,0,0,0,0,0});
  AM_   = std::vector<AMData>(bd_pars->Size(AM), {0,0,0});
  MM_   = std::vector<MMData>(bd_pars->Size(MM), 0);

  PAT_valid_  = std::vector<bool>(PAT_.size(), false);
  TAT0_valid_ = std::vector<bool>(TAT0_.size(), false);
  TAT1_valid_ = std::vector<bool>(TAT1_.size(), false);
  AM_valid_   = std::vector<bool>(AM_.size(), false);
  MM_valid_   = std::vector<bool>(MM_.size(), false);

  // initialize register vectors
  reg_ = std::vector<std::vector<unsigned int> >(bd_pars->NumReg(), {0});
  reg_valid_ = std::vector<bool>(reg_.size(), false);

}

BDState::~BDState()
{
}

void BDState::SetPAT(unsigned int start_addr, const std::vector<PATData> & data) {
  for (unsigned int i = 0; i < data.size(); i++) {
    PAT_[start_addr + i] = data[i];
    PAT_valid_[start_addr + i] = true;
  }
}

void BDState::SetTAT0(unsigned int start_addr, const std::vector<TATData> & data)
{
  for (unsigned int i = 0; i < data.size(); i++) {
    TAT0_[start_addr + i] = data[i];
    TAT0_valid_[start_addr + i] = true;
  }
}

void BDState::SetTAT1(unsigned int start_addr, const std::vector<TATData> & data)
{
  for (unsigned int i = 0; i < data.size(); i++) {
    TAT1_[start_addr + i] = data[i];
    TAT1_valid_[start_addr + i] = true;
  }
}

void BDState::SetAM(unsigned int start_addr, const std::vector<AMData> & data)
{
  for (unsigned int i = 0; i < data.size(); i++) {
    AM_[start_addr + i] = data[i];
    AM_valid_[start_addr + i] = true;
  }
}

void BDState::SetMM(unsigned int start_addr, const std::vector<MMData> & data)
{
  for (unsigned int i = 0; i < data.size(); i++) {
    MM_[start_addr + i] = data[i];
    MM_valid_[start_addr + i] = true;
  }
}

void BDState::SetReg(RegId reg_id, const std::vector<unsigned int> & data)
{
  reg_[reg_id] = data;
  reg_valid_[reg_id] = true;
}
  
const std::pair<const std::vector<unsigned int> *, bool> BDState::GetReg(RegId reg_id) const
{
  return make_pair(&(reg_.at(reg_id)), reg_valid_.at(reg_id));
}


void BDState::SetToggle(RegId reg_id, bool traffic_en, bool dump_en)
{
  bool already_off = AreTrafficRegsOff();

  SetReg(reg_id, {traffic_en, dump_en});

  if (AreTrafficRegsOff() & !already_off) { // if we just turned the last toggle off, set the timer
    all_traffic_off_start_ = std::chrono::high_resolution_clock::now();
  }
}

std::tuple<bool, bool, bool> BDState::GetToggle(RegId reg_id) const
{
  return std::make_tuple(reg_.at(reg_id)[0], reg_.at(reg_id)[1], reg_valid_.at(reg_id));
}

bool BDState::AreTrafficRegsOff() const 
/// is traffic_en == false for all traffic_regs_
/// note that !AreTrafficRegsOff() != (traffic_en == true for all traffic_regs_
{
  bool all_traffic_off = true;
  for(auto& reg_id : kTrafficRegs) {
    bool traffic_en, dump_en, reg_valid;
    std::tie(traffic_en, dump_en, reg_valid) = GetToggle(reg_id);

    all_traffic_off = all_traffic_off & !traffic_en & reg_valid;
  }
  return all_traffic_off;
}

bool BDState::IsTrafficOff() const 
/// has AreTrafficRegsOff been true for traffic_drain_us
{
  // check that traffic regs are still off
  if (AreTrafficRegsOff()) {
    auto time_since = std::chrono::high_resolution_clock::now() - all_traffic_off_start_;
    std::chrono::duration<unsigned int, std::micro> traffic_drain_time(driver_pars_->Get(bd_state_traffic_drain_us));
    if (time_since > traffic_drain_time) {
      return true;
    }
  }
  return false;
}

void BDState::WaitForTrafficOff() const
{
  while (!IsTrafficOff()) {}
}


} // bddriver
} // pystorm
