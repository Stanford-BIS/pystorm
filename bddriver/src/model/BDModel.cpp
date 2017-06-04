#include "BDModel.h"

namespace pystorm {
namespace bddriver {
namespace bdmodel {

BDModel::BDModel(const bdpars::BDPars* bd_pars, const driverpars::DriverPars* driver_pars) {
  driver_pars_ = driver_pars;
  bd_pars_     = bd_pars;

  state_ = new BDState(bd_pars_, driver_pars_);
}

BDModel::~BDModel() { delete state_; }

void BDModel::ParseInput(const std::vector<uint8_t>& input_stream) {
  // pack uint8_t stream into uint32_ts, other FPGA stuff
  std::vector<uint32_t> BD_input_words = FPGAInput(input_stream, bd_pars_);

  // perform horn decoding
  std::vector<std::vector<uint32_t> > horn_words = Horn(BD_input_words, bd_pars_);
  //cout << "did horn" << endl;
  //cout << "sizes:" << endl;
  //for (unsigned int i = 0; i < horn_words.size(); i++) {
  //  if (horn_words[i].size() > 0) cout << i << " : " << horn_words[i].size() << endl;
  //}

  // deserialize at horn leaves where required
  std::vector<std::vector<uint64_t> > des_horn_words = DeserializeAllHornLeaves(horn_words, bd_pars_);
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

std::vector<uint64_t> BDModel::Generate(bdpars::FunnelLeafId leaf_id) {
  using namespace bdpars;

  switch (leaf_id) {
    case RO_ACC: {
      VFieldValues vfv = DataToVFieldValues(acc_tags_to_send_);
      return Driver::PackWords(*bd_pars_->Word(ACC_OUTPUT_TAGS), vfv);
    }
    case RO_TAT: {
      VFieldValues vfv = DataToVFieldValues(TAT_tags_to_send_);
      return Driver::PackWords(*bd_pars_->Word(TAT_OUTPUT_TAGS), vfv);
    }
    case NRNI: {
      VFieldValues vfv = DataToVFieldValues(spikes_to_send_);
      return Driver::PackWords(*bd_pars_->Word(OUTPUT_SPIKES), vfv);
    }
    case DUMP_AM: {
      VFieldValues vfv = DataToVFieldValues(AM_dump_);
      return Driver::PackWords(*bd_pars_->Word(AM), vfv);
    }
    case DUMP_MM: {
      VFieldValues vfv = DataToVFieldValues(MM_dump_);
      return Driver::PackWords(*bd_pars_->Word(MM), vfv);
    }
    case DUMP_PAT: {
      VFieldValues vfv = DataToVFieldValues(PAT_dump_);
      return Driver::PackWords(*bd_pars_->Word(PAT), vfv);
    }
    case DUMP_TAT0: {
      VFieldValues vfv = DataToVFieldValues(TAT_dump_[0]);
      return Driver::PackWords(*bd_pars_->Word(TAT0), vfv);
    }
    case DUMP_TAT1: {
      VFieldValues vfv = DataToVFieldValues(TAT_dump_[1]);
      return Driver::PackWords(*bd_pars_->Word(TAT1), vfv);
    }
    case DUMP_PRE_FIFO: {
      VFieldValues vfv = DataToVFieldValues(pre_fifo_tags_to_send_);
      return Driver::PackWords(*bd_pars_->Word(PRE_FIFO_TAGS), vfv);
    }
    case DUMP_POST_FIFO0: {
      VFieldValues vfv = DataToVFieldValues(post_fifo_tags_to_send_[0]);
      return Driver::PackWords(*bd_pars_->Word(POST_FIFO_TAGS0), vfv);
    }
    case DUMP_POST_FIFO1: {
      VFieldValues vfv = DataToVFieldValues(post_fifo_tags_to_send_[1]);
      return Driver::PackWords(*bd_pars_->Word(POST_FIFO_TAGS1), vfv);
    }
    case OVFLW0: {
      assert(false && "not implemented");
    }
    case OVFLW1: {
      assert(false && "not implemented");
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
  std::vector<uint32_t> funnel_out_stream = Funnel(serialized_chunks_and_widths, bd_pars_);
  
  // then FPGA-byte-pack them
  return FPGAOutput(funnel_out_stream, bd_pars_);
}


std::pair<FieldValues, bdpars::MemWordId> BDModel::UnpackMemWordNWays(
    uint64_t input, std::vector<bdpars::MemWordId> words_to_try) {
  bool found = false;
  bdpars::MemWordId found_id;
  FieldValues found_field_vals;
  for (auto& word_id : words_to_try) {
    FieldValues field_vals = Driver::UnpackWord(*bd_pars_->Word(word_id), input);
    if (field_vals.size() > 0) {
      assert(!found && "undefined behavior for multiple matches");
      found_id         = word_id;
      found_field_vals = field_vals;
      found            = true;
    }
  }
  assert(found && "couldn't find a matching MemWord");
  return {found_field_vals, found_id};
}

void BDModel::ProcessReg(bdpars::RegId reg_id, uint64_t input) {
  const bdpars::WordStructure* reg_word_struct = bd_pars_->Word(reg_id);
  FieldValues field_vals                       = Driver::UnpackWord(*reg_word_struct, input);

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
  FieldValues field_vals                   = Driver::UnpackWord(*word_struct, input);

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
      assert(false && "not implemented");
      break;
    }
    case HT_FIFO_RESET: {
      assert(false && "not implemented");
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
  std::tie(field_vals, word_id) = UnpackMemWordNWays(input, {MM_SET_ADDRESS, MM_WRITE_INCREMENT, MM_READ_INCREMENT});

  switch (word_id) {
    case MM_SET_ADDRESS:
      MM_address_ = FVGet(field_vals, ADDRESS);
      break;

    case MM_WRITE_INCREMENT: {
      FieldValues data_fields = Driver::UnpackWord(*bd_pars_->Word(MM), FVGet(field_vals, DATA));
      state_->SetMM(MM_address_, {FieldValuesToMMData(data_fields)});
      MM_address_++;
      break;
    }
    case MM_READ_INCREMENT:
      MM_dump_.push_back(state_->GetMM()->at(MM_address_));
      MM_address_++;
      break;

    default:
      assert(false && "bad FVV");
  }
}

void BDModel::ProcessAM(uint64_t input) {
  using namespace bdpars;

  FieldValues field_vals;
  MemWordId word_id;
  std::tie(field_vals, word_id) = UnpackMemWordNWays(input, {AM_SET_ADDRESS, AM_READ_WRITE, AM_INCREMENT});

  switch (word_id) {
    case AM_SET_ADDRESS:
      AM_address_ = FVGet(field_vals, ADDRESS);
      // cout << "SET " << AM_address_ << endl;
      break;

    case AM_READ_WRITE: {
      AM_dump_.push_back(state_->GetAM()->at(AM_address_));
      FieldValues data_fields = Driver::UnpackWord(*bd_pars_->Word(AM), FVGet(field_vals, DATA));
      state_->SetAM(AM_address_, {FieldValuesToAMData(data_fields)});
      // cout << "WRITE AT " << AM_address_ << " : ";
      // for (auto& it : data_fields) {
      //  cout << "(" << it.first << ":" << it.second << "), ";
      //}
      // cout << endl;
      break;
    }
    case AM_INCREMENT:
      AM_address_++;
      // cout << "INC " << AM_address_ << endl;
      break;

    default:
      assert(false && "bad FVV");
  }
}

void BDModel::ProcessTAT(unsigned int TAT_idx, uint64_t input) {
  using namespace bdpars;

  FieldValues field_vals;
  MemWordId word_id;
  std::tie(field_vals, word_id) = UnpackMemWordNWays(input, {TAT_SET_ADDRESS, TAT_READ_INCREMENT, TAT_WRITE_INCREMENT});

  switch (word_id) {
    case TAT_SET_ADDRESS:
      TAT_address_[TAT_idx] = FVGet(field_vals, ADDRESS);
      break;

    case TAT_WRITE_INCREMENT: {
      // for TAT, have to try all three data word types
      for (unsigned int i = 0; i < 3; i++) {
        FieldValues data_fields = Driver::UnpackWord(*bd_pars_->Word(TAT0, i), FVGet(field_vals, DATA));
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
  std::tie(field_vals, word_id) = UnpackMemWordNWays(input, {PAT_READ, PAT_WRITE});

  switch (word_id) {
    case PAT_READ:
      PAT_dump_.push_back(state_->GetPAT()->at(FVGet(field_vals, ADDRESS)));
      break;

    case PAT_WRITE: {
      FieldValues data_fields = Driver::UnpackWord(*bd_pars_->Word(PAT), FVGet(field_vals, DATA));
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
          std::tie(field_vals, word_id) = UnpackMemWordNWays(input, {AM_ENCAPSULATION, MM_ENCAPSULATION});
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
