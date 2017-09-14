#ifndef BDMODEL_H
#define BDMODEL_H

#include <cstdint>
#include <array>
#include <vector>
#include <mutex>

#include "BDModelUtil.h"
#include "common/BDPars.h"
#include "common/BDState.h"
#include "common/DriverPars.h"
#include "common/DriverTypes.h"
#include "common/binary_util.h"

namespace pystorm {
namespace bddriver {
namespace bdmodel {

/// BDModel pretends to be the BD hardware.
/// Public ifc is threadsafe.
class BDModel {
 public:
  BDModel(const bdpars::BDPars* bd_pars, const driverpars::DriverPars* driver_pars);
  ~BDModel();

  /// parse input stream to update internal BDState object and other state
  void ParseInput(const std::vector<uint8_t>& input_stream);
  /// given internal state, generate requested output stream
  std::vector<uint8_t> GenerateOutputs();

  // calls that will cause the model to emit some traffic, exercising upstream driver calls
  inline void PushOutput(bdpars::OutputId output_id, const std::vector<BDWord> & to_append) {
      std::unique_lock<std::mutex>(mutex_);
      bdpars::BDStartPointId leaf_id = bd_pars_->BDStartPointIdFor(output_id);
      to_send_.at(leaf_id).insert(to_send_.at(leaf_id).end(), to_append.begin(), to_append.end()); 
  }

  // calls to retrieve the results of downstream driver calls

  /// lock the model, get a const ptr to the state, examine it as you like...
  inline const BDState* LockState() 
    { mutex_.lock(); return state_; }

  /// then unlock the model when you're done
  inline void UnlockState()
    { mutex_.unlock(); }
  
  // XXX alternative to these two calls would be GetState which would create a copy of the state, return that

  inline std::vector<BDWord> PopSpikes() 
    { std::unique_lock<std::mutex>(mutex_); 
      std::vector<BDWord> recvd = std::move(received_spikes_);
      received_spikes_.clear();
      return recvd; }

  inline std::vector<BDWord> PopTags() 
    { std::unique_lock<std::mutex>(mutex_); 
      std::vector<BDWord> recvd = std::move(received_tags_);
      received_tags_.clear();
      return recvd; }

 private:

  /// Mutex used by public calls to make BDModel thread-safe.
  /// e.g. user may have a worker thread continuously call ParseInput(...); ... = GenerateOutputs();
  /// while supplying upstream traffic with PushUpstreamX() and popping outputs with PopX() 
  /// from another thread.
  std::mutex mutex_;
  
  /// primary internal state object, same as maintained by driver
  BDState* state_;

  // BDState doesn't capture all hardware state, additional state objects
  
  // final results of downstream calls, must be dequeued by user

  std::vector<BDWord> received_spikes_; /// received downstream spikes
  std::vector<BDWord> received_tags_; /// received downstream tags

  // intermediate results of downstream/upstream calls, will be sent by a future GenerateOutputs()

  std::array<std::vector<BDWord>, bdpars::BDStartPointIdCount> to_send_;

  // upstream traffic enqueued by the user, will be sent by a future GenerateOutputs()

  // word remainders that couldn't be completely deserialized
  std::array<std::vector<uint32_t>, bdpars::BDEndPointIdCount> remainders_;

  // memory address register states

  unsigned int MM_address_     = 0;
  unsigned int AM_address_     = 0;
  unsigned int TAT_address_[2] = {0, 0};

  // input parameters to state_
  
  const driverpars::DriverPars* driver_pars_;
  const bdpars::BDPars* bd_pars_;

  // used in ParseInput, once words have been horn-decoded and deserialized
  
  void Process(bdpars::BDEndPointId leaf_id, const std::vector<uint32_t>& inputs);
  void ProcessInput(bdpars::BDEndPointId leaf_id, uint32_t input);
  void ProcessReg(bdpars::RegId reg_id, uint32_t input);
  void ProcessInput(bdpars::InputId input_id, uint32_t input);
  void ProcessMM(uint32_t input);
  void ProcessAM(uint32_t input);
  void ProcessTAT(unsigned int TAT_idx, uint32_t input);
  void ProcessPAT(uint32_t input);

  // used in GenerateOutputs
  
  std::vector<uint32_t> Generate(bdpars::BDStartPointId leaf_id);

  // more complicated than the other cases
  std::vector<uint32_t> GenerateTAT(unsigned int tat_idx);

  /// helper for Process calls (just to look up funnel id)
  inline void PushMem(bdpars::MemId mem_id, const std::vector<BDWord> & to_append) {
      bdpars::BDStartPointId leaf_id = bd_pars_->BDStartPointIdFor(mem_id);
      to_send_.at(leaf_id).insert(to_send_.at(leaf_id).end(), to_append.begin(), to_append.end()); 
  }

};

}  // bdmodel
}  // bddriver
}  // pystorm

#endif
