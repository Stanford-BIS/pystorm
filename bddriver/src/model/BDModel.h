#ifndef BDMODEL_H
#define BDMODEL_H

#include <cstdint>
#include <vector>

#include "BDModelUtil.h"
#include "common/BDPars.h"
#include "common/BDState.h"
#include "common/DriverPars.h"
#include "common/DriverTypes.h"
#include "common/binary_util.h"

namespace pystorm {
namespace bddriver {
namespace bdmodel {

class BDModel {
 public:
  BDModel(const bdpars::BDPars* bd_pars, const driverpars::DriverPars* driver_pars);
  ~BDModel();

  /// parse input stream to update internal BDState object and other state
  void ParseInput(const std::vector<uint8_t>& input_stream);
  /// given internal state, generate requested output stream
  std::vector<uint8_t> GenerateOutputs();

  inline void AddUpstreamSpikes(const std::vector<NrnSpike> & spikes) 
    { spikes_to_send_.insert(spikes_to_send_.end(), spikes.begin(), spikes.end()); }
  inline void AddUpstreamTATTags(const std::vector<Tag> & tags) 
    { TAT_tags_to_send_.insert(TAT_tags_to_send_.end(), tags.begin(), tags.end()); }
  inline void AddUpstreamAccTags(const std::vector<Tag> & tags) 
    { acc_tags_to_send_.insert(acc_tags_to_send_.end(), tags.begin(), tags.end()); }
  inline void AddPreFIFOTags(const std::vector<Tag> & tags)
    { pre_fifo_tags_to_send_.insert(pre_fifo_tags_to_send_.end(), tags.begin(), tags.end()); }
  inline void AddPostFIFOTags(const std::vector<Tag> & tags, unsigned int idx)
    { post_fifo_tags_to_send_[idx].insert(post_fifo_tags_to_send_[idx].end(), tags.begin(), tags.end()); }
  inline void AddFIFOOverflow(unsigned int num_to_add, unsigned int idx)
    { n_ovflw_to_send_[idx] += num_to_add; }

  inline const BDState* GetState() { return state_; }
  inline const std::vector<SynSpike>* GetSpikes() { return &received_spikes_; }
  inline const std::vector<Tag>* GetTags() { return &received_tags_; }

 private:
  // updated by the Process call
  BDState* state_;

  std::vector<SynSpike> received_spikes_;
  std::vector<Tag> received_tags_;

  // intermediate results

  std::vector<Tag> TAT_tags_to_send_;
  std::vector<Tag> acc_tags_to_send_;
  std::vector<Tag> pre_fifo_tags_to_send_;
  std::vector<Tag> post_fifo_tags_to_send_[2];
  std::vector<NrnSpike> spikes_to_send_;
  std::vector<MMData> MM_dump_;
  std::vector<AMData> AM_dump_;
  std::vector<PATData> PAT_dump_;
  std::vector<TATData> TAT_dump_[2];
  unsigned int n_ovflw_to_send_[2];

  unsigned int MM_address_     = 0;
  unsigned int AM_address_     = 0;
  unsigned int TAT_address_[2] = {0, 0};

  // needed by state_
  const driverpars::DriverPars* driver_pars_;
  const bdpars::BDPars* bd_pars_;

  // useful helper for the mem processing calls
  std::pair<FieldValues, bdpars::MemWordId> UnpackMemWordNWays(
      uint64_t input, std::vector<bdpars::MemWordId> words_to_try);

  void Process(bdpars::HornLeafId leaf_id, const std::vector<uint64_t>& inputs);
  void ProcessInput(bdpars::HornLeafId leaf_id, uint64_t input);
  void ProcessReg(bdpars::RegId reg_id, uint64_t input);
  void ProcessInput(bdpars::InputId input_id, uint64_t input);
  void ProcessMM(uint64_t input);
  void ProcessAM(uint64_t input);
  void ProcessTAT(unsigned int TAT_idx, uint64_t input);
  void ProcessPAT(uint64_t input);

  std::vector<uint64_t> Generate(bdpars::FunnelLeafId leaf_id);
};

}  // bdmodel
}  // bddriver
}  // pystorm

#endif
