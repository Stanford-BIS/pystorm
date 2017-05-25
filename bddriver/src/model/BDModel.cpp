#include "BDModel.h"

namespace pystorm {
namespace bddriver {
namespace bdmodel {

BDModel::BDModel(const bdpars::BDPars * bd_pars, const driverpars::DriverPars * driver_pars)
{
  driver_pars_ = driver_pars;
  bd_pars_ = bd_pars;

  state_ = new BDState(bd_pars_, driver_pars_);
}

BDModel::~BDModel()
{
  delete state_;
}

void BDModel::ParseInput(const std::vector<uint8_t> & input_stream)
{
  // pack uint8_t stream into uint32_ts, other FPGA stuff
  std::vector<uint32_t> BD_input_words = FPGAInput(input_stream, bd_pars_);

  // perform horn decoding
  std::vector<std::vector<uint32_t> > horn_words = Horn(BD_input_words, bd_pars_);

  // deserialize at horn leaves where required
  std::vector<std::vector<uint64_t> > des_horn_words = DeserializeAllHornLeaves(horn_words, bd_pars_);

  // iterate through leaves 
  for (unsigned int i = 0; i < des_horn_words.size(); i++) {
    auto leaf_id = static_cast<bdpars::HornLeafId>(i);
    std::vector<FieldVValues> leaf_vfvv = ParseHornLeaf(des_horn_words[i], leaf_id, bd_pars_);

    Process(leaf_vfvv);
  }
}

void BDModel::ProcessReg(bdpars::RegId reg_id, const std::vector<FieldVValues> & inputs) {
  assert(inputs.size() == 1 && "only one word type per register");
  assert(FVVExactlyMatchesWordStruct(inputs.back(), *bd_pars_->Word(reg_id)));

  // if we set it multiple times, just take the last one
  std::vector<FieldValues> vfv = VFVVasVFV(inputs);
  FieldValues last_set_field_vals = vfv.back();

  const bdpars::WordStructure * reg_word_struct = bd_pars_->Word(reg_id);

  // form vector of values to set BDState's reg state with, in WordStructure field order
  std::vector<unsigned int> field_vals_as_vect;
  for (auto& it : *reg_word_struct) {
    bdpars::WordFieldId field_id = it.first;
    field_vals_as_vect.push_back(last_set_field_vals.at(field_id));
  }
  state_->SetReg(reg_id, field_vals_as_vect);

}

void BDModel::ProcessInput(bdpars::InputId input_id, const std::vector<FieldVValues> & inputs) {
  using namespace bdpars;
  assert(inputs.size() == 1 && "only one word type per inputs");
  FieldVValues fvv = inputs.back();
  assert(FVVExactlyMatchesWordStruct(fvv, *bd_pars_->Word(input_id)));
  std::vector<unsigned int> zeros(fvv.begin()->second.size(), 0);

  switch (input_id) {
    case INPUT_TAGS :
    {
      std::vector<Tag> new_inputs = FieldVValuesToTag(fvv, zeros, zeros); // XXX times and core ids are zero
      received_tags_.insert(received_tags_.end(), new_inputs.begin(), new_inputs.end()); // concatenate
    }
    case INPUT_SPIKES :
    {
      std::vector<SynSpike> new_inputs = FieldVValuesToSynSpike(fvv, zeros, zeros); // XXX times and core ids are zero
      received_spikes_.insert(received_spikes_.end(), new_inputs.begin(), new_inputs.end()); // concatenate
    }
    case DCT_FIFO_INPUT_TAGS :
    {
      assert(false && "not implemented");
      break;
    }
    case HT_FIFO_RESET :
    {
      assert(false && "not implemented");
      break;
    }
    case TILE_SRAM_INPUTS :
    {
      assert(false && "not implemented");
      break;
    }
    default :
    {
      assert(false);
      break;
    }
  }
}

void BDModel::ProcessMM(const std::vector<FieldVValues> & inputs)
{
  using namespace bdpars;

  for (const FieldVValues& input : inputs) {
    if (FVVContainsWordStruct(input, *bd_pars_->Word(MM_SET_ADDRESS))) {
      std::vector<uint64_t> addr_vals = input.at(ADDRESS);
      MM_address_ = addr_vals.back(); // take the last one
    } else if (FVVContainsWordStruct(input, *bd_pars_->Word(MM_WRITE_INCREMENT))) {
      std::vector<MMData> MM_data = FieldVValuesToMMData(input);
      state_->SetMM(MM_address_, MM_data);
    } else if (FVVContainsWordStruct(input, *bd_pars_->Word(MM_READ_INCREMENT))) {
      MM_dump_.push_back(state_->GetMM()->at(MM_address_));
    } else {
      assert(false && "bad FVV");
    }
  }
}

void BDModel::ProcessAM(const std::vector<FieldVValues> & inputs)
{
  using namespace bdpars;

  for (const FieldVValues& input : inputs) {
    if (FVVContainsWordStruct(input, *bd_pars_->Word(AM_SET_ADDRESS))) {
      std::vector<uint64_t> addr_vals = input.at(ADDRESS);
      AM_address_ = addr_vals.back(); // take the last one
    } else if (FVVContainsWordStruct(input, *bd_pars_->Word(AM_READ_WRITE))) {
      AM_dump_.push_back(state_->GetAM()->at(AM_address_));
      std::vector<AMData> AM_data = FieldVValuesToAMData(input);
      state_->SetAM(AM_address_, AM_data);
    } else if (FVVContainsWordStruct(input, *bd_pars_->Word(AM_INCREMENT))) {
      AM_address_++;
    } else {
      assert(false && "bad FVV");
    }
  }
}

void BDModel::ProcessTAT(unsigned int TAT_idx, const std::vector<FieldVValues> & inputs)
{
  using namespace bdpars;

  for (const FieldVValues& input : inputs) {
    if (FVVContainsWordStruct(input, *bd_pars_->Word(TAT_SET_ADDRESS))) {
      std::vector<uint64_t> addr_vals = input.at(ADDRESS);
      TAT_address_[TAT_idx] = addr_vals.back(); // take the last one
    } else if (FVVContainsWordStruct(input, *bd_pars_->Word(TAT_WRITE_INCREMENT))) {
      std::vector<TATData> TAT_data = FieldVValuesToTATData(FVVasVFV(input));
      if(TAT_idx == 0) {
        state_->SetTAT0(TAT_address_[TAT_idx], TAT_data);
      } else {
        state_->SetTAT1(TAT_address_[TAT_idx], TAT_data);
      }
    } else if (FVVContainsWordStruct(input, *bd_pars_->Word(TAT_READ_INCREMENT))) {
      TATData read_data;
      if(TAT_idx == 0) {
        read_data = state_->GetTAT0()->at(TAT_address_[TAT_idx]);
      } else {
        read_data = state_->GetTAT1()->at(TAT_address_[TAT_idx]);
      }
      TAT_dump_[TAT_idx].push_back(read_data);
    } else {
      assert(false && "bad FVV");
    }
  }
}

void BDModel::ProcessPAT(const std::vector<FieldVValues> & inputs)
{
  using namespace bdpars;

  for (const FieldVValues& input : inputs) {
    if (FVVContainsWordStruct(input, *bd_pars_->Word(PAT_READ))) {
      std::vector<uint64_t> addr_vals = input.at(ADDRESS);
      for (auto& addr : addr_vals) {
        PAT_dump_.push_back(state_->GetPAT()->at(addr));
      }
    } else if (FVVContainsWordStruct(input, *bd_pars_->Word(PAT_WRITE))) {
      std::vector<uint64_t> addr_vals = input.at(ADDRESS);
      std::vector<PATData> PAT_data = FieldVValuesToPATData(input);
      for (auto& addr : addr_vals) {
        state_->SetPAT(addr, PAT_data);
      }
    } else {
      assert(false && "bad FVV");
    }
  }
}

void BDModel::Process(const std::vector<FieldVValues> & inputs)
{
  using namespace bdpars;

  for (unsigned int i = 0; i < inputs.size(); i++) {

    auto leaf_id = static_cast<bdpars::HornLeafId>(i);

    ComponentTypeId component_type = bd_pars_->ComponentTypeIdFor(leaf_id);
    unsigned int component_idx = bd_pars_->ComponentIdxFor(leaf_id);

    switch (component_type) {
      case REG :
      {
        RegId reg_id = static_cast<RegId>(component_idx);
        ProcessReg(reg_id, inputs);
      }

      // reg and input are basically the same
      case INPUT :
      {
        InputId input_id = static_cast<InputId>(component_idx);
        return ProcessInput(input_id, inputs);
      }

      // memory is complicated =(
      case MEM :
      {
        MemId mem_id = static_cast<MemId>(component_idx);

        switch (mem_id) {
          case MM : // actually this catches AM too
          {
            return ProcessMM(inputs);
          }
          case PAT :
          {
            return ProcessPAT(inputs);
          }
          case TAT0 :
          {
            return ProcessTAT(0, inputs);
          }
          case TAT1 :
          {
            return ProcessTAT(1, inputs);
          }
          default :
          {
            assert(false);
          }
        }
      }

      default :
      {
        assert(false);
      }
    }

  }
}

} // bdmodel
} // bddriver
} // pystorm
