#include "BDState.h"

#include <assert.h>
#include <chrono>
#include <thread>
#include <vector>
#include <unordered_map>

#include "BDPars.h"

namespace pystorm {
namespace bddriver {

#include <iostream>
using std::cout;
using std::endl;

BDState::BDState(const bdpars::BDPars* bd_pars, const driverpars::DriverPars* driver_pars) {
  bd_pars_     = bd_pars;
  driver_pars_ = driver_pars;

  // initialize memory vectors
  mems_[bdpars::BDMemId::PAT]  = std::vector<BDWord>(bd_pars->mem_info_[bdpars::BDMemId::PAT],  BDWord(0));
  mems_[bdpars::BDMemId::TAT0] = std::vector<BDWord>(bd_pars->mem_info_[bdpars::BDMemId::TAT0], BDWord(0));
  mems_[bdpars::BDMemId::TAT1] = std::vector<BDWord>(bd_pars->mem_info_[bdpars::BDMemId::TAT1], BDWord(0));
  mems_[bdpars::BDMemId::AM]   = std::vector<BDWord>(bd_pars->mem_info_[bdpars::BDMemId::AM],   BDWord(0));
  mems_[bdpars::BDMemId::MM]   = std::vector<BDWord>(bd_pars->mem_info_[bdpars::BDMemId::MM],   BDWord(0));

  mems_valid_[bdpars::BDMemId::PAT]  = std::vector<bool>(mems_[bdpars::BDMemId::PAT].size(),  false);
  mems_valid_[bdpars::BDMemId::TAT0] = std::vector<bool>(mems_[bdpars::BDMemId::TAT0].size(), false);
  mems_valid_[bdpars::BDMemId::TAT1] = std::vector<bool>(mems_[bdpars::BDMemId::TAT1].size(), false);
  mems_valid_[bdpars::BDMemId::AM]   = std::vector<bool>(mems_[bdpars::BDMemId::AM].size(),   false);
  mems_valid_[bdpars::BDMemId::MM]   = std::vector<bool>(mems_[bdpars::BDMemId::MM].size(),   false);

  // initialize register vectors
  for (unsigned int i = 0; i < bdpars::BDHornEPCount; i++) {
    reg_[i] = BDWord(0);
    reg_valid_[i] = false;
  }

  // Valid config values are 0 or 1. 2 => uninitialized
  soma_config_mem_ = std::vector<std::map<bdpars::ConfigSomaID, std::vector<unsigned int>>>(256, {
    {bdpars::ConfigSomaID::GAIN_0, std::vector<unsigned int>(16, 2)},
    {bdpars::ConfigSomaID::GAIN_1, std::vector<unsigned int>(16, 2)},
    {bdpars::ConfigSomaID::OFFSET_0, std::vector<unsigned int>(16, 2)},
    {bdpars::ConfigSomaID::OFFSET_1, std::vector<unsigned int>(16, 2)},
    {bdpars::ConfigSomaID::ENABLE, std::vector<unsigned int>(16, 2)},
    {bdpars::ConfigSomaID::SUBTRACT_OFFSET, std::vector<unsigned int>(16, 2)}
  });

  synapse_config_mem_ = std::vector<std::map<bdpars::ConfigSynapseID, std::vector<unsigned int>>>(256, {
    {bdpars::ConfigSynapseID::SYN_DISABLE, std::vector<unsigned int>(4, 2)},
    {bdpars::ConfigSynapseID::ADC_DISABLE, std::vector<unsigned int>(4, 2)},
  });

  diffusor_config_mem_ = std::vector<std::map<bdpars::DiffusorCutLocationId, std::vector<unsigned int>>>(256, {
    {bdpars::DiffusorCutLocationId::NORTH_LEFT, std::vector<unsigned int>(1, 2)},
    {bdpars::DiffusorCutLocationId::NORTH_RIGHT, std::vector<unsigned int>(1, 2)},
    {bdpars::DiffusorCutLocationId::WEST_TOP, std::vector<unsigned int>(1, 2)},
    {bdpars::DiffusorCutLocationId::WEST_BOTTOM, std::vector<unsigned int>(1, 2)},
  });
}

BDState::~BDState() {}

void BDState::SetMem(bdpars::MemId mem_id, unsigned int start_addr, const std::vector<BDWord> &data) {
  for (unsigned int i = 0; i < data.size(); i++) {
    mems_.at(mem_id).at(start_addr + i) = data[i];
    mems_valid_.at(mem_id).at(start_addr + i) = true;
  }
}

void BDState::SetReg(bdpars::BDHornEP reg_id, BDWord data) {
  // we could have just set a traffic toggle
  bool already_off = AreTrafficRegsOff();

  reg_[reg_id]       = data;
  reg_valid_[reg_id] = true;

  if (AreTrafficRegsOff() & !already_off) {  // if we just turned the last toggle off, set the timer
    all_traffic_off_start_ = std::chrono::high_resolution_clock::now();
  }
}

const std::pair<const BDWord *, bool> BDState::GetReg(bdpars::BDHornEP reg_id) const {
  return std::make_pair(&(reg_.at(reg_id)), reg_valid_.at(reg_id));
}

void BDState::SetToggle(bdpars::BDHornEP reg_id, bool traffic_en, bool dump_en) {
  BDWord toggle_word = BDWord::Create<ToggleWord>({{ToggleWord::TRAFFIC_ENABLE, traffic_en}, {ToggleWord::DUMP_ENABLE, dump_en}});
  SetReg(reg_id, toggle_word);
}

std::tuple<bool, bool, bool> BDState::GetToggle(bdpars::BDHornEP reg_id) const
/// Returns traffic_en, dump_en, valid
{
  BDWord word = reg_.at(reg_id);
  return std::make_tuple(word.At<ToggleWord>(ToggleWord::TRAFFIC_ENABLE), 
                         word.At<ToggleWord>(ToggleWord::DUMP_ENABLE),
                         reg_valid_.at(reg_id));
}

bool BDState::AreTrafficRegsOff() const
/// is traffic_en == false for all traffic_regs_
/// note that !AreTrafficRegsOff() != (traffic_en == true) for all traffic_regs_
/// because not every register may be programmed yet (all registers not valid)
{
  bool all_traffic_off = true;
  for (auto& reg_id : kTrafficRegs) {
    bool traffic_en, dump_en, reg_valid;
    std::tie(traffic_en, dump_en, reg_valid) = GetToggle(reg_id);

    all_traffic_off = all_traffic_off & !traffic_en & reg_valid;
  }
  return all_traffic_off;
}

int BDState::TrafficRegWaitTimeLeftUs() const {
  auto time_since = std::chrono::high_resolution_clock::now() - all_traffic_off_start_;
  std::chrono::duration<unsigned int, std::micro> traffic_drain_time(
      driver_pars_->Get(driverpars::BD_STATE_TRAFFIC_DRAIN_US));
  std::chrono::duration<int, std::micro> wait_time_left =
      std::chrono::duration_cast<std::chrono::microseconds>(traffic_drain_time - time_since);
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

void BDState::WaitForTrafficOff() const {
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
bool operator==(const BDState& lhs, const BDState& rhs)
// kind of a pain because of all the structs
{
  // comparison of vectors works as expected if the comparators for the underlying stored objects is defined.

  // check memories
  bool AM_matches = *lhs.GetMem(bdpars::AM) == *rhs.GetMem(bdpars::AM);
  // if (!AM_matches) cout << "AM didn't match" << endl;
  bool MM_matches = *lhs.GetMem(bdpars::MM) == *rhs.GetMem(bdpars::MM);
  // if (!MM_matches) cout << "MM didn't match" << endl;
  bool TAT0_matches = *lhs.GetMem(bdpars::TAT0) == *rhs.GetMem(bdpars::TAT0);
  // if (!TAT0_matches) cout << "TAT0 didn't match" << endl;
  bool TAT1_matches = *lhs.GetMem(bdpars::TAT1) == *rhs.GetMem(bdpars::TAT1);
  // if (!TAT1_matches) cout << "TAT1 didn't match" << endl;
  bool PAT_matches = *lhs.GetMem(bdpars::PAT) == *rhs.GetMem(bdpars::PAT);
  // if (!PAT_matches) cout << "PAT didn't match" << endl;
  bool mems_match = AM_matches && MM_matches && TAT0_matches && TAT1_matches && PAT_matches;

  // check registers
  bool regs_match = true;
  for (unsigned int i = 0; i < bdpars::BDHornEPCount; i++) {
    const BDWord *lhs_vals, *rhs_vals;
    bool lhs_valid, rhs_valid;
    std::tie(lhs_vals, lhs_valid) = lhs.GetReg(static_cast<bdpars::BDHornEP>(i));
    std::tie(rhs_vals, rhs_valid) = rhs.GetReg(static_cast<bdpars::BDHornEP>(i));
    bool valid_match = lhs_valid == rhs_valid;
    bool vals_match  = *lhs_vals == *rhs_vals;
    regs_match       = regs_match && valid_match && vals_match;
    // if (!valid_match) {
    //  cout << "reg id " << i << " valid failed" << endl;
    //  cout << lhs_valid << " vs " << rhs_valid << endl;
    //}
    // if (!vals_match) {
    //  cout << "reg id " << i << " vals failed" << endl;
    //  cout << "size " << lhs_vals->size() << " vs " << rhs_vals->size() << endl;
    //  for (unsigned int i = 0; i < lhs_vals->size(); i++) {
    //    cout << lhs_vals->at(i) << " vs " << rhs_vals->at(i) << endl;
    //  }
    //}
  }

  return mems_match && regs_match;
}

}  // bddriver
}  // pystorm
