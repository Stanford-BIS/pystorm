#include "BDState.h"

#include <chrono>
#include <thread>
#include <vector>
#include <assert.h>

#include "BDPars.h"

namespace pystorm {
namespace bddriver {

#include <iostream>
using std::cout;
using std::endl;

BDState::BDState(const bdpars::BDPars * bd_pars, const driverpars::DriverPars * driver_pars)
{

  bd_pars_ = bd_pars;
  driver_pars_ = driver_pars;

  // initialize memory vectors
  PAT_  = std::vector<PATData>(bd_pars->Size(bdpars::PAT), {0,0,0});
  TAT0_ = std::vector<TATData>(bd_pars->Size(bdpars::TAT0), {0,0,0,0,0,0,0,0,0,0});
  TAT1_ = std::vector<TATData>(bd_pars->Size(bdpars::TAT1), {0,0,0,0,0,0,0,0,0,0});
  AM_   = std::vector<AMData>(bd_pars->Size(bdpars::AM), {0,0,0});
  MM_   = std::vector<MMData>(bd_pars->Size(bdpars::MM), 0);

  PAT_valid_  = std::vector<bool>(PAT_.size(), false);
  TAT0_valid_ = std::vector<bool>(TAT0_.size(), false);
  TAT1_valid_ = std::vector<bool>(TAT1_.size(), false);
  AM_valid_   = std::vector<bool>(AM_.size(), false);
  MM_valid_   = std::vector<bool>(MM_.size(), false);

  // initialize register vectors
  for (unsigned int i = 0; i < bd_pars->NumReg(); i++) {
    unsigned int num_reg_fields = bd_pars->Word(static_cast<bdpars::RegId>(i))->size();
    reg_.push_back(std::vector<unsigned int>(num_reg_fields, 0));
  }
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

void BDState::SetReg(bdpars::RegId reg_id, const std::vector<unsigned int> & data)
{
  // we could have just set a traffic toggle
  bool already_off = AreTrafficRegsOff();

  reg_[reg_id] = data;
  reg_valid_[reg_id] = true;

  if (AreTrafficRegsOff() & !already_off) { // if we just turned the last toggle off, set the timer
    all_traffic_off_start_ = std::chrono::high_resolution_clock::now();
  }
}
  
const std::pair<const std::vector<unsigned int> *, bool> BDState::GetReg(bdpars::RegId reg_id) const
{
  return make_pair(&(reg_.at(reg_id)), reg_valid_.at(reg_id));
}


void BDState::SetToggle(bdpars::RegId reg_id, bool traffic_en, bool dump_en)
{
  // just to make sure we've hardcoded the right order
  const bdpars::WordStructure * toggle_word_struct = bd_pars_->Word(bdpars::NEURON_DUMP_TOGGLE);
  assert(toggle_word_struct->at(0).first == bdpars::TRAFFIC_ENABLE);
  assert(toggle_word_struct->at(1).first == bdpars::DUMP_ENABLE);

  SetReg(reg_id, {traffic_en, dump_en});
}

std::tuple<bool, bool, bool> BDState::GetToggle(bdpars::RegId reg_id) const
/// Returns traffic_en, dump_en, valid
{
  // just to make sure we've hardcoded the right order
  const bdpars::WordStructure * toggle_word_struct = bd_pars_->Word(bdpars::NEURON_DUMP_TOGGLE);
  assert(toggle_word_struct->at(0).first == bdpars::TRAFFIC_ENABLE);
  assert(toggle_word_struct->at(1).first == bdpars::DUMP_ENABLE);

  return std::make_tuple(reg_.at(reg_id)[0], reg_.at(reg_id)[1], reg_valid_.at(reg_id));
}

bool BDState::AreTrafficRegsOff() const 
/// is traffic_en == false for all traffic_regs_
/// note that !AreTrafficRegsOff() != (traffic_en == true) for all traffic_regs_ 
/// because not every register may be programmed yet (all registers not valid)
{
  bool all_traffic_off = true;
  for(auto& reg_id : kTrafficRegs) {
    bool traffic_en, dump_en, reg_valid;
    std::tie(traffic_en, dump_en, reg_valid) = GetToggle(reg_id);

    all_traffic_off = all_traffic_off & !traffic_en & reg_valid;
  }
  return all_traffic_off;
}

int BDState::TrafficRegWaitTimeLeftUs() const
{
  auto time_since = std::chrono::high_resolution_clock::now() - all_traffic_off_start_;
  std::chrono::duration<unsigned int, std::micro> traffic_drain_time(driver_pars_->Get(driverpars::BD_STATE_TRAFFIC_DRAIN_US));
  std::chrono::duration<int, std::micro> wait_time_left = std::chrono::duration_cast<std::chrono::microseconds>(traffic_drain_time - time_since);
  return wait_time_left.count();
}

bool BDState::IsTrafficOff() const 
/// has AreTrafficRegsOff been true for traffic_drain_us
{
  // check that traffic regs are still off
  if (AreTrafficRegsOff()) {
    if (TrafficRegWaitTimeLeftUs() <= 0) {
      return true;
    }
  }
  return false;
}

void BDState::WaitForTrafficOff() const
{
  while (!IsTrafficOff()) {  
    int time_left = TrafficRegWaitTimeLeftUs();
    if (time_left > 0) {
      std::this_thread::sleep_for(std::chrono::microseconds(time_left));
    }
  }
}

/// Compare BDStates to see if all the fields match.
/// Useful for testing when using BDModel. Can compare Driver's 
/// BDState to the BDModels BDState
bool operator==(const BDState & lhs, const BDState & rhs)
// kind of a pain because of all the structs
{
  // comparison of vectors works as expected if the comparators for the underlying stored objects is defined.
  
  // check memories
  bool AM_matches = *lhs.GetAM() == *rhs.GetAM();
  //if (!AM_matches) cout << "AM didn't match" << endl;
  bool MM_matches = *lhs.GetMM() == *rhs.GetMM();
  //if (!MM_matches) cout << "MM didn't match" << endl;
  bool TAT0_matches = *lhs.GetTAT0() == *rhs.GetTAT0();
  //if (!TAT0_matches) cout << "TAT0 didn't match" << endl;
  bool TAT1_matches = *lhs.GetTAT1() == *rhs.GetTAT1();
  //if (!TAT1_matches) cout << "TAT1 didn't match" << endl;
  bool PAT_matches = *lhs.GetPAT() == *rhs.GetPAT();
  //if (!PAT_matches) cout << "PAT didn't match" << endl;
  bool mems_match = AM_matches && MM_matches && TAT0_matches && TAT1_matches && PAT_matches;

  // check registers
  bool regs_match = true;
  for (unsigned int i = 0; i < bdpars::LastRegId; i++) {
    const std::vector<unsigned int> * lhs_vals, * rhs_vals;
    bool lhs_valid, rhs_valid;
    std::tie(lhs_vals, lhs_valid) = lhs.GetReg(static_cast<bdpars::RegId>(i));
    std::tie(rhs_vals, rhs_valid) = rhs.GetReg(static_cast<bdpars::RegId>(i));
    bool valid_match = lhs_valid == rhs_valid;
    bool vals_match = *lhs_vals == *rhs_vals;
    regs_match = regs_match && valid_match && vals_match;
    //if (!valid_match) {
    //  cout << "reg id " << i << " valid failed" << endl;
    //  cout << lhs_valid << " vs " << rhs_valid << endl;
    //}
    //if (!vals_match) {
    //  cout << "reg id " << i << " vals failed" << endl;
    //  cout << "size " << lhs_vals->size() << " vs " << rhs_vals->size() << endl;
    //  for (unsigned int i = 0; i < lhs_vals->size(); i++) {
    //    cout << lhs_vals->at(i) << " vs " << rhs_vals->at(i) << endl;
    //  }
    //}
  }

  return mems_match && regs_match;
  
}


} // bddriver
} // pystorm
