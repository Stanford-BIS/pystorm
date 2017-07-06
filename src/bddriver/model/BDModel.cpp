#include "BDModel.h"

#include <vector>
#include <array>

#include "common/BDPars.h"

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

  std::unique_lock<std::mutex>(mutex_);

  // pack uint8_t stream into uint32_ts, other FPGA stuff
  std::vector<uint32_t> BD_input_words = FPGAInput(input_stream, bd_pars_);

  // perform horn decoding
  std::array<std::vector<uint32_t>, bdpars::HornLeafIdCount> new_horn_words = Horn(BD_input_words, bd_pars_);
  //cout << "did horn" << endl;
  //cout << "sizes:" << endl
  //for (unsigned int i = 0; i < horn_words.size(); i++) {
  //  if (horn_words[i].size() > 0) cout << i << " : " << horn_words[i].size() << endl;
  //}

  // deserialize at horn leaves where required
  std::array<std::vector<uint32_t>, bdpars::HornLeafIdCount> horn_words;
  horn_words.swap(remainders_);
  for (unsigned int i = 0 ; i < new_horn_words.size(); i++) {
    horn_words.at(i).insert(horn_words.at(i).end(), new_horn_words.at(i).begin(), new_horn_words.at(i).end());
  }

  std::array<std::vector<uint64_t>, bdpars::HornLeafIdCount> des_horn_words;
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


std::vector<uint8_t> BDModel::GenerateOutputs() {

  // convert to_send_ to uints XXX should be able to streamline this
  std::array<std::vector<uint64_t>, bdpars::FunnelLeafIdCount> packed_words;
  for (unsigned int i = 0; i < to_send_.size(); i++) {
    for (auto& it : to_send_.at(i)) {
      packed_words.at(i).push_back(it.Packed());
    }
    to_send_.at(i).clear();
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
  state_->SetReg(reg_id, BDWord(input));
}

void BDModel::ProcessInput(bdpars::InputId input_id, uint64_t input) {
  using namespace bdpars;

  switch (input_id) {
    case INPUT_TAGS: {
      //cout << "got tag" << endl;
      //cout << FVGet(field_vals, TAG) << ", " << FVGet(field_vals, COUNT) << endl;
      received_tags_.push_back(BDWord(input));
      break;
    }
    case INPUT_SPIKES: {
      received_spikes_.push_back(BDWord(input));
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

  BDWord word(input);

  if (word.At<MMSetAddress>(MMSetAddress::FIXED_0) == 0) {
    MM_address_ = word.At<MMSetAddress>(MMSetAddress::ADDRESS);
    //cout << "SET " << MM_address_ << endl;

  } else if (word.At<MMWriteIncrement>(MMWriteIncrement::FIXED_1) == 1) {
    uint64_t data = word.At<MMWriteIncrement>(MMWriteIncrement::DATA);
    state_->SetMem(bdpars::MM, MM_address_, {BDWord(data)});
    //cout << "WRITE INC " << MM_address_ << " : " << data << endl;;
    MM_address_++;

  } else if (word.At<MMReadIncrement>(MMReadIncrement::FIXED_2) == 2) {
    BDWord data = state_->GetMem(bdpars::MM)->at(MM_address_);
    PushMem(bdpars::MM, {data});
    //cout << "READ INC " << MM_address_ << " : " << data.Packed() << endl;;
    MM_address_++;

  } else { 
    assert(false);
  }
}

void BDModel::ProcessAM(uint64_t input) {
  using namespace bdpars;

  BDWord word(input);

  if (word.At<AMSetAddress>(AMSetAddress::FIXED_0) == 0) {
    AM_address_ = word.At<AMSetAddress>(AMSetAddress::ADDRESS);
    //cout << "SET " << AM_address_ << endl;

  } else if (word.At<AMReadWrite>(AMReadWrite::FIXED_1) == 1) {
    BDWord old_data = state_->GetMem(bdpars::AM)->at(AM_address_);
    PushMem(bdpars::AM, {old_data});

    uint64_t new_data = word.At<AMReadWrite>(AMReadWrite::DATA);
    state_->SetMem(bdpars::AM, AM_address_, {BDWord(new_data)});
    //cout << "READ/WRITE AT " << AM_address_ << " : ";
    // for (auto& it : data_fields) {
    //  cout << "(" << it.first << ":" << it.second << "), ";
    //}
    //cout << endl;

  } else if (word.At<AMIncrement>(AMIncrement::FIXED_2) == 2) {
      AM_address_++;
      //cout << "INC " << AM_address_ << endl;

  } else { 
    assert(false);
  }
}

void BDModel::ProcessTAT(unsigned int TAT_idx, uint64_t input) {
  using namespace bdpars;

  BDWord word(input);

  if (word.At<TATSetAddress>(TATSetAddress::FIXED_0) == 0) {
    TAT_address_[TAT_idx] = word.At<TATSetAddress>(TATSetAddress::ADDRESS);

  } else if (word.At<TATWriteIncrement>(TATWriteIncrement::FIXED_1) == 1) {
    uint64_t data = word.At<TATWriteIncrement>(TATWriteIncrement::DATA);
    if (TAT_idx == 0) {
      state_->SetMem(bdpars::TAT0, TAT_address_[0], {BDWord(data)});
    } else {
      state_->SetMem(bdpars::TAT1, TAT_address_[1], {BDWord(data)});
    }
    TAT_address_[TAT_idx]++;

  } else if (word.At<TATReadIncrement>(TATReadIncrement::FIXED_2) == 2) {
    BDWord data(0);
    if (TAT_idx == 0) {
      data = state_->GetMem(bdpars::TAT0)->at(TAT_address_[0]);
      PushMem(bdpars::TAT0, {data});
    } else {
      data = state_->GetMem(bdpars::TAT0)->at(TAT_address_[1]);
      PushMem(bdpars::TAT1, {data});
    }
    TAT_address_[TAT_idx]++;

  } else { 
    assert(false);
  }
}

void BDModel::ProcessPAT(const uint64_t input) {
  using namespace bdpars;

  BDWord word(input);

  if (word.At<PATWrite>(PATWrite::FIXED_0) == 0) {
      unsigned int addr = word.At<PATWrite>(PATWrite::ADDRESS);
      uint64_t data = word.At<PATWrite>(PATWrite::DATA);
      state_->SetMem(bdpars::PAT, addr, {BDWord(data)});

  } else if (word.At<PATRead>(PATRead::FIXED_1) == 1) {
      unsigned int addr = word.At<PATRead>(PATRead::ADDRESS);
      BDWord data = state_->GetMem(bdpars::PAT)->at(addr);
      PushMem(bdpars::PAT, {data});
      //cout << "PAT READ : " << addr << endl;

  } else {
    assert(false);
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
          BDWord word(input);
          if (word.At<AMEncapsulation>(AMEncapsulation::FIXED_0) == 0) {
            ProcessAM(word.At<AMEncapsulation>(AMEncapsulation::PAYLOAD));
          } else if (word.At<MMEncapsulation>(MMEncapsulation::FIXED_1) == 1) {
            ProcessMM(word.At<MMEncapsulation>(MMEncapsulation::PAYLOAD));
          } else { 
            assert(false);
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
