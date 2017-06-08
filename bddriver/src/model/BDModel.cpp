#include "BDModel.h"

namespace pystorm {
namespace bddriver {
namespace bdmodel {

BDModel::BDModel(const bdpars::BDPars* bd_pars, const driverpars::DriverPars* driver_pars) {
  driver_pars_ = driver_pars;
  bd_pars_     = bd_pars;

  state_ = new BDState(bd_pars_, driver_pars_);

  remainders_ = std::vector<std::vector<uint32_t> >(bdpars::LastHornLeafId+1, std::vector<uint32_t>());
  n_ovflw_to_send_[0] = 0;
  n_ovflw_to_send_[1] = 0;
}

BDModel::~BDModel() { delete state_; }

void BDModel::ParseInput(const std::vector<uint8_t>& input_stream) {

  std::unique_lock<std::mutex>(mutex_);

  // pack uint8_t stream into uint32_ts, other FPGA stuff
  std::vector<uint32_t> BD_input_words = FPGAInput(input_stream, bd_pars_);

  // perform horn decoding
  std::vector<std::vector<uint32_t> > new_horn_words = Horn(BD_input_words, bd_pars_);
  //cout << "did horn" << endl;
  //cout << "sizes:" << endl;
  //for (unsigned int i = 0; i < horn_words.size(); i++) {
  //  if (horn_words[i].size() > 0) cout << i << " : " << horn_words[i].size() << endl;
  //}

  // deserialize at horn leaves where required
  std::vector<std::vector<uint32_t> > horn_words;
  horn_words.swap(remainders_);
  for (unsigned int i = 0 ; i < new_horn_words.size(); i++) {
    horn_words.at(i).insert(horn_words.at(i).end(), new_horn_words.at(i).begin(), new_horn_words.at(i).end());
  }

  std::vector<std::vector<uint64_t> > des_horn_words;
  std::tie(des_horn_words, remainders_) = DeserializeAllHornLeaves(horn_words, bd_pars_);

  //cout << "did des" << endl;
  //cout << "sizes: " << endl;
  //for (unsigned int i = 0; i < des_horn_words.size(); i++) {
  //  if (des_horn_words[i].size() > 0) cout << i << " : " << des_horn_words[i].size() << endl;
  //}
  //for(unsigned int i = 0 ; i < des_horn_words.size(); i++) {
  //  for (auto& it : des_horn_words[i]) {
  //    cout << it << endl;
  //  }
  //}

  // iterate through leaves
  for (unsigned int i = 0; i < des_horn_words.size(); i++) {
    auto leaf_id = static_cast<bdpars::HornLeafId>(i);

    // cout << "processing leaf " << i << endl;
    // if (des_horn_words[i].size() > 0) cout << "  it has something" << endl;

    Process(leaf_id, des_horn_words[i]);
  }
}

// helper for Generate
std::vector<uint64_t> BDModel::GenerateTAT(unsigned int tat_idx) {
  VFieldValues vfv = DataToVFieldValues(TAT_dump_[tat_idx]);
  TAT_dump_[tat_idx].clear();
  std::vector<uint64_t> retval;
  for (auto& fv : vfv) {
    unsigned int TAT_word_type = 0;
    if (FVContains(fv, bdpars::AM_ADDRESS))             TAT_word_type = 0;
    else if (FVContains(fv, bdpars::SYNAPSE_ADDRESS_0)) TAT_word_type = 1;
    else if (FVContains(fv, bdpars::TAG))               TAT_word_type = 2;
    else assert(false);
    retval.push_back(PackWord(*bd_pars_->Word(bdpars::TAT0, TAT_word_type), fv));
  }
  return retval;
}

std::vector<uint64_t> BDModel::Generate(bdpars::FunnelLeafId leaf_id) {
  // XXX this is pretty damn repetitive: template?
  using namespace bdpars;

  std::unique_lock<std::mutex>(mutex_);

  switch (leaf_id) {
    case RO_ACC: {
      VFieldValues vfv = DataToVFieldValues(acc_tags_to_send_);
      acc_tags_to_send_.clear();
      return PackWords(*bd_pars_->Word(ACC_OUTPUT_TAGS), vfv);
    }
    case RO_TAT: {
      VFieldValues vfv = DataToVFieldValues(TAT_tags_to_send_);
      TAT_tags_to_send_.clear();
      return PackWords(*bd_pars_->Word(TAT_OUTPUT_TAGS), vfv);
    }
    case NRNI: {
      VFieldValues vfv = DataToVFieldValues(spikes_to_send_);
      spikes_to_send_.clear();
      return PackWords(*bd_pars_->Word(OUTPUT_SPIKES), vfv);
    }
    case DUMP_AM: {
      VFieldValues vfv = DataToVFieldValues(AM_dump_);
      AM_dump_.clear();
      return PackWords(*bd_pars_->Word(AM), vfv);
    }
    case DUMP_MM: {
      VFieldValues vfv = DataToVFieldValues(MM_dump_);
      MM_dump_.clear();
      return PackWords(*bd_pars_->Word(MM), vfv);
    }
    case DUMP_PAT: {
      VFieldValues vfv = DataToVFieldValues(PAT_dump_);
      PAT_dump_.clear();
      return PackWords(*bd_pars_->Word(PAT), vfv);
    }
    case DUMP_TAT0: {
      return GenerateTAT(0);
    }
    case DUMP_TAT1: {
      return GenerateTAT(1);
    }
    case DUMP_PRE_FIFO: {
      VFieldValues vfv = DataToVFieldValues(pre_fifo_tags_to_send_);
      pre_fifo_tags_to_send_.clear();
      return PackWords(*bd_pars_->Word(PRE_FIFO_TAGS), vfv);
    }
    case DUMP_POST_FIFO0: {
      VFieldValues vfv = DataToVFieldValues(post_fifo_tags_to_send_[0]);
      post_fifo_tags_to_send_[0].clear();
      return PackWords(*bd_pars_->Word(POST_FIFO_TAGS0), vfv);
    }
    case DUMP_POST_FIFO1: {
      VFieldValues vfv = DataToVFieldValues(post_fifo_tags_to_send_[1]);
      post_fifo_tags_to_send_[1].clear();
      return PackWords(*bd_pars_->Word(POST_FIFO_TAGS1), vfv);
    }
    case OVFLW0: {
      VFieldValues vfv= std::vector<FieldValues>(n_ovflw_to_send_[0], {{}});
      n_ovflw_to_send_[0] = 0;
      return PackWords(*bd_pars_->Word(OVERFLOW_TAGS0), vfv);
    }
    case OVFLW1: {
      VFieldValues vfv= std::vector<FieldValues>(n_ovflw_to_send_[1], {{}});
      n_ovflw_to_send_[1] = 0;
      return PackWords(*bd_pars_->Word(OVERFLOW_TAGS0), vfv);
    }
    default: {
      assert(false);
    }
  }
}

std::vector<uint8_t> BDModel::GenerateOutputs() {

  // get packed words for each funnel leaf feeder
  std::vector<std::vector<uint64_t> > packed_words;
  for (unsigned int i = 0; i < bdpars::LastFunnelLeafId+1; i++) {
    auto leaf_id = static_cast<bdpars::FunnelLeafId>(i);
    packed_words.push_back(Generate(leaf_id));
  }

  // serialize if necessary (AM word only?)
  auto serialized_chunks_and_widths = SerializeAllFunnelLeaves(packed_words, bd_pars_);

  // then funnel-encode them
  std::vector<uint64_t> funnel_out_stream = Funnel(serialized_chunks_and_widths, bd_pars_);
  
  // then FPGA-byte-pack them
  std::vector<uint8_t> FPGA_output = FPGAOutput(funnel_out_stream, bd_pars_);

  return FPGA_output;
}


void BDModel::ProcessReg(bdpars::RegId reg_id, uint64_t input) {
  const bdpars::WordStructure* reg_word_struct = bd_pars_->Word(reg_id);
  FieldValues field_vals                       = UnpackWord(*reg_word_struct, input);

  // form vector of values to set BDState's reg state with, in WordStructure field order
  std::vector<unsigned int> field_vals_as_vect;
  for (auto& it : *reg_word_struct) {
    bdpars::WordFieldId field_id = it.first;
    field_vals_as_vect.push_back(FVGet(field_vals, field_id));
  }
  state_->SetReg(reg_id, field_vals_as_vect);
}

void BDModel::ProcessInput(bdpars::InputId input_id, uint64_t input) {
  using namespace bdpars;

  const bdpars::WordStructure* word_struct = bd_pars_->Word(input_id);
  FieldValues field_vals                   = UnpackWord(*word_struct, input);

  switch (input_id) {
    case INPUT_TAGS: {
      //cout << "got tag" << endl;
      //cout << FVGet(field_vals, TAG) << ", " << FVGet(field_vals, COUNT) << endl;
      Tag new_input = FieldValuesToTag(field_vals, 0, 0);  // XXX times and core ids are zero
      received_tags_.push_back(new_input);
      break;
    }
    case INPUT_SPIKES: {
      SynSpike new_input = FieldValuesToSynSpike(field_vals, 0, 0);  // XXX times and core ids are zero
      received_spikes_.push_back(new_input);
      break;
    }
    case DCT_FIFO_INPUT_TAGS: {
      // nothing to do. Not tracking state for this
      break;
    }
    case HT_FIFO_RESET: {
      // nothing to do. Not tracking state for this
      break;
    }
    case TILE_SRAM_INPUTS: {
      assert(false && "not implemented");
      break;
    }
    default: {
      assert(false);
      break;
    }
  }
}

void BDModel::ProcessMM(uint64_t input) {
  using namespace bdpars;

  FieldValues field_vals;
  MemWordId word_id;
  std::tie(field_vals, word_id) = UnpackMemWordNWays(input, {MM_SET_ADDRESS, MM_WRITE_INCREMENT, MM_READ_INCREMENT}, bd_pars_);

  switch (word_id) {
    case MM_SET_ADDRESS:
      MM_address_ = FVGet(field_vals, ADDRESS);
      //cout << "SET " << MM_address_ << endl;
      break;

    case MM_WRITE_INCREMENT: {
      FieldValues data_fields = UnpackWord(*bd_pars_->Word(MM), FVGet(field_vals, DATA));
      state_->SetMM(MM_address_, {FieldValuesToMMData(data_fields)});
      //cout << "WRITE INC " << MM_address_ << " : ";
      //for (auto& it : data_fields) {
      //  cout << "(" << it.first << ":" << it.second << "), ";
      //}
      //cout << endl;
      MM_address_++;
      break;
    }
    case MM_READ_INCREMENT: {
      MMData data = state_->GetMM()->at(MM_address_);
      MM_dump_.push_back(data);
      //cout << "READ INC " << MM_address_ << " : ";
      //for (auto& it : DataToFieldValues(data)) {
      //  cout << "(" << it.first << ":" << it.second << "), ";
      //}
      //cout << endl;
      MM_address_++;
      break;
    }
    default:
      assert(false && "bad FVV");
  }
}

void BDModel::ProcessAM(uint64_t input) {
  using namespace bdpars;

  FieldValues field_vals;
  MemWordId word_id;
  std::tie(field_vals, word_id) = UnpackMemWordNWays(input, {AM_SET_ADDRESS, AM_READ_WRITE, AM_INCREMENT}, bd_pars_);

  switch (word_id) {
    case AM_SET_ADDRESS:
      AM_address_ = FVGet(field_vals, ADDRESS);
      //cout << "SET " << AM_address_ << endl;
      break;

    case AM_READ_WRITE: {
      AM_dump_.push_back(state_->GetAM()->at(AM_address_));
      FieldValues data_fields = UnpackWord(*bd_pars_->Word(AM), FVGet(field_vals, DATA));
      state_->SetAM(AM_address_, {FieldValuesToAMData(data_fields)});
      //cout << "READ/WRITE AT " << AM_address_ << " : ";
      // for (auto& it : data_fields) {
      //  cout << "(" << it.first << ":" << it.second << "), ";
      //}
      //cout << endl;
      break;
    }
    case AM_INCREMENT:
      AM_address_++;
      //cout << "INC " << AM_address_ << endl;
      break;

    default:
      assert(false && "bad FVV");
  }
}

void BDModel::ProcessTAT(unsigned int TAT_idx, uint64_t input) {
  using namespace bdpars;

  FieldValues field_vals;
  MemWordId word_id;
  std::tie(field_vals, word_id) = UnpackMemWordNWays(input, {TAT_SET_ADDRESS, TAT_READ_INCREMENT, TAT_WRITE_INCREMENT}, bd_pars_);

  switch (word_id) {
    case TAT_SET_ADDRESS:
      TAT_address_[TAT_idx] = FVGet(field_vals, ADDRESS);
      break;

    case TAT_WRITE_INCREMENT: {
      // for TAT, have to try all three data word types
      for (unsigned int i = 0; i < 3; i++) {
        FieldValues data_fields = UnpackWord(*bd_pars_->Word(TAT0, i), FVGet(field_vals, DATA));
        if (data_fields.size() > 0) {
          if (TAT_idx == 0) {
            state_->SetTAT0(TAT_address_[TAT_idx], {FieldValuesToTATData(data_fields)});
          } else {
            state_->SetTAT1(TAT_address_[TAT_idx], {FieldValuesToTATData(data_fields)});
          }
          TAT_address_[TAT_idx]++;
        }
      }
      break;
    }
    case TAT_READ_INCREMENT:
      if (TAT_idx == 0) {
        TAT_dump_[TAT_idx].push_back(state_->GetTAT0()->at(TAT_address_[TAT_idx]));
      } else {
        TAT_dump_[TAT_idx].push_back(state_->GetTAT1()->at(TAT_address_[TAT_idx]));
      }
      TAT_address_[TAT_idx]++;
      break;

    default:
      assert(false && "bad FVV");
  }
}

void BDModel::ProcessPAT(const uint64_t input) {
  using namespace bdpars;

  FieldValues field_vals;
  MemWordId word_id;
  std::tie(field_vals, word_id) = UnpackMemWordNWays(input, {PAT_READ, PAT_WRITE}, bd_pars_);

  switch (word_id) {
    case PAT_READ:
      PAT_dump_.push_back(state_->GetPAT()->at(FVGet(field_vals, ADDRESS)));
      break;

    case PAT_WRITE: {
      FieldValues data_fields = UnpackWord(*bd_pars_->Word(PAT), FVGet(field_vals, DATA));
      state_->SetPAT(FVGet(field_vals, ADDRESS), {FieldValuesToPATData(data_fields)});
      break;
    }
    default:
      assert(false && "bad FVV");
  }
}

void BDModel::Process(bdpars::HornLeafId leaf_id, const std::vector<uint64_t>& inputs) {
  for (auto& input : inputs) {
    ProcessInput(leaf_id, input);
  }
}

void BDModel::ProcessInput(bdpars::HornLeafId leaf_id, uint64_t input) {
  using namespace bdpars;

  ComponentTypeId component_type = bd_pars_->ComponentTypeIdFor(leaf_id);
  unsigned int component_idx     = bd_pars_->ComponentIdxFor(leaf_id);

  switch (component_type) {
    case REG: {
      RegId reg_id = static_cast<RegId>(component_idx);
      ProcessReg(reg_id, input);
      break;
    }

    // reg and input are basically the same
    case INPUT: {
      InputId input_id = static_cast<InputId>(component_idx);
      ProcessInput(input_id, input);
      break;
    }

    // memory is complicated =(
    case MEM: {
      MemId mem_id = static_cast<MemId>(component_idx);

      switch (mem_id) {
        case MM:  // actually this catches AM too
        {
          FieldValues field_vals;
          MemWordId word_id;
          std::tie(field_vals, word_id) = UnpackMemWordNWays(input, {AM_ENCAPSULATION, MM_ENCAPSULATION}, bd_pars_);
          // cout << UintAsString(input, 64) << " to " << endl;
          // for (auto& it : field_vals) {
          //  cout << "(" << it.first << ":" << it.second << "), ";
          //}
          if (word_id == AM_ENCAPSULATION) {
            ProcessAM(FVGet(field_vals, PAYLOAD));
          } else {
            ProcessMM(FVGet(field_vals, PAYLOAD));
          }
          break;
        }
        case PAT: {
          ProcessPAT(input);
          break;
        }
        case TAT0: {
          ProcessTAT(0, input);
          break;
        }
        case TAT1: {
          ProcessTAT(1, input);
          break;
        }
        default: { assert(false); }
      }
      break;
    }

    default: { assert(false); }
  }
}

}  // bdmodel
}  // bddriver
}  // pystorm
