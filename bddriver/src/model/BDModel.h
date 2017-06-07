#ifndef BDMODEL_H
#define BDMODEL_H

#include <cstdint>
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

  inline void PushUpstreamSpikes(const std::vector<NrnSpike> & spikes) 
    { std::unique_lock<std::mutex>(mutex_);
      spikes_to_send_.insert(spikes_to_send_.end(), spikes.begin(), spikes.end()); }

  inline void PushUpstreamTATTags(const std::vector<Tag> & tags) 
    { std::unique_lock<std::mutex>(mutex_);
      TAT_tags_to_send_.insert(TAT_tags_to_send_.end(), tags.begin(), tags.end()); }

  inline void PushUpstreamAccTags(const std::vector<Tag> & tags) 
    { std::unique_lock<std::mutex>(mutex_);
      acc_tags_to_send_.insert(acc_tags_to_send_.end(), tags.begin(), tags.end()); }

  inline void PushPreFIFOTags(const std::vector<Tag> & tags)
    { std::unique_lock<std::mutex>(mutex_);
      pre_fifo_tags_to_send_.insert(pre_fifo_tags_to_send_.end(), tags.begin(), tags.end()); }

  inline void PushPostFIFOTags(const std::vector<Tag> & tags, unsigned int idx)
    { std::unique_lock<std::mutex>(mutex_);
      post_fifo_tags_to_send_[idx].insert(post_fifo_tags_to_send_[idx].end(), tags.begin(), tags.end()); }

  inline void PushFIFOOverflow(unsigned int num_to_add, unsigned int idx)
    { std::unique_lock<std::mutex>(mutex_);
      n_ovflw_to_send_[idx] += num_to_add; }

  // calls to retrieve the results of downstream driver calls

  /// lock the model, get a const ptr to the state, examine it as you like...
  inline const BDState* LockState() 
    { mutex_.lock(); return state_; }

  /// then unlock the model when you're done
  inline void UnlockState()
    { mutex_.unlock(); }
  
  // XXX alternative to these two calls would be GetState which would create a copy of the state, return that

  inline std::vector<SynSpike> PopSpikes() 
    { std::unique_lock<std::mutex>(mutex_); 
      std::vector<SynSpike> recvd = std::move(received_spikes_);
      received_spikes_.clear();
      return recvd; }

  inline std::vector<Tag> PopTags() 
    { std::unique_lock<std::mutex>(mutex_); 
      std::vector<Tag> recvd = std::move(received_tags_);
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

  std::vector<SynSpike> received_spikes_; /// received downstream spikes
  std::vector<Tag> received_tags_; /// received downstream tags

  // intermediate results of downstream/upstream calls, will be sent by a future GenerateOutputs()

  std::vector<MMData> MM_dump_; /// memory dump requested by user's dump call 
  std::vector<AMData> AM_dump_; /// memory dump requested by user's dump call 
  std::vector<PATData> PAT_dump_; /// memory dump requested by user's dump call 
  std::vector<TATData> TAT_dump_[2]; /// memory dump requested by user's dump call 

  // upstream traffic enqueued by the user, will be sent by a future GenerateOutputs()

  std::vector<Tag> TAT_tags_to_send_; /// upstream TAT tags to send, enqueued by user
  std::vector<Tag> acc_tags_to_send_; /// upstream acc tags to send, enqueued by user
  std::vector<Tag> pre_fifo_tags_to_send_; /// upstream pre fifo tags to send, enqueued by user
  std::vector<Tag> post_fifo_tags_to_send_[2]; /// upstream post fifo tags to send, enqueued by user
  std::vector<NrnSpike> spikes_to_send_; /// upstream spikes to send, enqueued by user
  unsigned int n_ovflw_to_send_[2]; /// upstream overflow warnings to send, enqueued by user

  // word remainders that couldn't be completely deserialized
  std::vector<std::vector<uint32_t> > remainders_;

  // memory address register states

  unsigned int MM_address_     = 0;
  unsigned int AM_address_     = 0;
  unsigned int TAT_address_[2] = {0, 0};

  // input parameters to state_
  
  const driverpars::DriverPars* driver_pars_;
  const bdpars::BDPars* bd_pars_;

  // used in ParseInput, once words have been horn-decoded and deserialized
  
  void Process(bdpars::HornLeafId leaf_id, const std::vector<uint64_t>& inputs);
  void ProcessInput(bdpars::HornLeafId leaf_id, uint64_t input);
  void ProcessReg(bdpars::RegId reg_id, uint64_t input);
  void ProcessInput(bdpars::InputId input_id, uint64_t input);
  void ProcessMM(uint64_t input);
  void ProcessAM(uint64_t input);
  void ProcessTAT(unsigned int TAT_idx, uint64_t input);
  void ProcessPAT(uint64_t input);

  // used in GenerateOutputs
  
  std::vector<uint64_t> Generate(bdpars::FunnelLeafId leaf_id);

  // more complicated than the other cases
  std::vector<uint64_t> GenerateTAT(unsigned int tat_idx);
};

}  // bdmodel
}  // bddriver
}  // pystorm

#endif
