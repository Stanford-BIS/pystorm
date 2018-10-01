#ifndef BDSTATE_H
#define BDSTATE_H

#include <chrono>
#include <string>
#include <vector>
#include <unordered_map>

#include "BDPars.h"
#include "BDWord.h"
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
  BDState(const bdpars::BDPars *bd_pars);
  ~BDState();

  void SetMem(bdpars::BDMemId mem_id, unsigned int start_addr, const std::vector<BDWord> &data);
  inline const std::vector<BDWord> *GetMem(bdpars::BDMemId mem_id) const { return &mems_.at(mem_id); }

  void SetReg(bdpars::BDHornEP reg_id, BDWord data);
  const std::pair<const BDWord, bool> GetReg(bdpars::BDHornEP reg_id) const;


  void SetNeuronConfigMem(unsigned int core_id,
                          unsigned int tile_id,
                          unsigned int elem_id,
                          bdpars::ConfigSomaID config_type,
                          unsigned int config_value){
    soma_config_mem_[tile_id][config_type][elem_id] = config_value;
  }

  std::vector<std::map<bdpars::ConfigSomaID, std::vector<unsigned int>>> GetSomaConfigMem() const { return soma_config_mem_; }

  void SetNeuronConfigMem(unsigned int core_id,
                          unsigned int tile_id,
                          unsigned int elem_id,
                          bdpars::ConfigSynapseID config_type,
                          unsigned int config_value){
    synapse_config_mem_[tile_id][config_type][elem_id] = config_value;
  }

  std::vector<std::map<bdpars::ConfigSynapseID, std::vector<unsigned int>>> GetSynapseConfigMem() const { return synapse_config_mem_; }

  void SetNeuronConfigMem(unsigned int core_id,
                          unsigned int tile_id,
                          unsigned int elem_id,
                          bdpars::DiffusorCutLocationId config_type,
                          unsigned int config_value){
    diffusor_config_mem_[tile_id][config_type][elem_id] = config_value;
  }

  std::vector<std::map<bdpars::DiffusorCutLocationId, std::vector<unsigned int>>> GetDiffusorConfigMem() const { return diffusor_config_mem_; }

  // A toggle is a special case of register
  void SetToggle(bdpars::BDHornEP reg_id, bool traffic_en, bool dump_en);
  std::tuple<bool, bool, bool> GetToggle(bdpars::BDHornEP reg_id) const;

  bool AreTrafficRegsOff() const;  /// is traffic_en == false for all traffic_regs_
  bool IsTrafficOff() const;       /// has AreTrafficRegsOff been true for traffic_drain_us
  void WaitForTrafficOff() const;  /// Busy wait until IsTrafficOff()

  // only private so we can get to it in the == operator function
  const bdpars::BDPars *bd_pars_;

  bool CompareTo(const BDState & other_state_obj) const;

 private:

  // register contents
  std::unordered_map<bdpars::BDHornEP, BDWord, EnumClassHash> reg_;
  std::unordered_map<bdpars::BDHornEP, bool, EnumClassHash>   reg_valid_;

  // memory contents
  std::unordered_map<bdpars::BDMemId, std::vector<BDWord>, EnumClassHash> mems_; // conceptually, should be map of different-sized arrays

  // binary vectors denote whether memory entries have been programmed
  std::unordered_map<bdpars::BDMemId, std::vector<bool>, EnumClassHash> mems_valid_;

  //////////////////////////////////////////////////////////////////////
  /// Neuron state
  //////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////
  /// Soma State Memory
  //////////////////////////////////////////////////////////////////////
  // soma_config_mem_[tile_id][bdpars::ConfigSomaID][soma_id]
  std::vector<std::map<bdpars::ConfigSomaID, std::vector<unsigned int>>> soma_config_mem_;

  //////////////////////////////////////////////////////////////////////
  /// Synapse State Memory
  //////////////////////////////////////////////////////////////////////
  // synapse_config_mem_[tile_id][bdpars::ConfigSynapseID][synapse_id]
  std::vector<std::map<bdpars::ConfigSynapseID, std::vector<unsigned int>>> synapse_config_mem_;

  //////////////////////////////////////////////////////////////////////
  /// Diffusor State Memory
  //////////////////////////////////////////////////////////////////////
  // diffusor_config_mem_[tile_id][bdpars::DiffusorCutLocationId][0]
  std::vector<std::map<bdpars::DiffusorCutLocationId, std::vector<unsigned int>>> diffusor_config_mem_;

  // I iterate through these a lot, this is for convenience
  const std::vector<bdpars::BDHornEP> kTrafficRegs = {
    bdpars::BDHornEP::NEURON_DUMP_TOGGLE, 
    bdpars::BDHornEP::TOGGLE_PRE_FIFO,
    bdpars::BDHornEP::TOGGLE_POST_FIFO0, 
    bdpars::BDHornEP::TOGGLE_POST_FIFO1};

  // timing: when certain things happened
  std::chrono::high_resolution_clock::time_point all_traffic_off_start_;

  int TrafficRegWaitTimeLeftUs() const;
};

// equality test, checks all fields
bool operator==(const BDState &lhs, const BDState &rhs);

}  // bddriver
}  // pystorm

#endif
