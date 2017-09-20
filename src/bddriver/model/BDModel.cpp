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

  // decode endpoint
  std::unordered_map<uint8_t, std::vector<uint32_t>> new_horn_words;
  for (auto& it : BD_input_words) {
    uint8_t code = GetField<FPGAIO>(it, FPGAIO::EP_CODE);
    uint32_t payload = GetField<FPGAIO>(it, FPGAIO::PAYLOAD);
    new_horn_words.at(code).push_back(payload);
  }

  // deserialize at horn leaves where required
  
  // build up starting from remainders of last iteration
  std::unordered_map<uint8_t, std::vector<uint32_t>> horn_words;
  horn_words.swap(remainders_);
  for (unsigned int i = 0 ; i < new_horn_words.size(); i++) {
    horn_words.at(i).insert(horn_words.at(i).end(), new_horn_words.at(i).begin(), new_horn_words.at(i).end());
  }

  std::unordered_map<uint8_t, std::vector<uint64_t>> des_horn_words;
  std::tie(des_horn_words, remainders_) = DeserializeAllCodes(horn_words, bd_pars_);

  // iterate through leaves
  for (auto& it : des_horn_words) {
    uint8_t code = it.first;
    Process(code, it.second);
  }
}


std::vector<uint8_t> BDModel::GenerateOutputs() {

  // serialize if necessary (AM word only?)
  std::unordered_map<uint8_t, std::pair<std::vector<uint64_t>, unsigned int>> 
    serialized_chunks_and_widths = SerializeAllCodes(to_send_, bd_pars_);

  // then pack into FPGA words
  std::vector<uint32_t> FPGA_words;
  for (auto& it : serialized_chunks_and_widths) {
    uint8_t code = it.first;
    assert(it.second.second < 24); // serialized data width must be less than FPGA payload size
    for (auto& payload : it.second.first) {
      FPGA_words.push_back(PackWord<FPGAIO>({{FPGAIO::EP_CODE, code}, {FPGAIO::PAYLOAD, payload}}));
    }
  }
  
  // then FPGA-byte-pack them
  std::vector<uint8_t> FPGA_output = FPGAOutput(FPGA_words, bd_pars_);

  // clear to_send_
  for (auto& it : to_send_) {
    it.second.clear();
  }

  return FPGA_output;
}


void BDModel::ProcessBDReg(bdpars::BDHornEP reg_id, uint64_t input) {
  assert(bd_pars_->BDHornEPIsReg(reg_id));
  state_->SetReg(reg_id, BDWord(input));
}

void BDModel::ProcessBDInputStream(bdpars::BDHornEP input_id, uint64_t input) {
  using namespace bdpars;

  switch (input_id) {
    case BDHornEP::RI: {
      //cout << "got tag" << endl;
      //cout << FVGet(field_vals, TAG) << ", " << FVGet(field_vals, COUNT) << endl;
      received_tags_.push_back(BDWord(input));
      break;
    }
    case BDHornEP::NEURON_INJECT: {
      received_spikes_.push_back(BDWord(input));
      break;
    }
    case BDHornEP::INIT_FIFO_DCT: {
      // nothing to do. Not tracking state for this
      break;
    }
    case BDHornEP::INIT_FIFO_HT: {
      // nothing to do. Not tracking state for this
      break;
    }
    case BDHornEP::NEURON_CONFIG: {
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

  if (GetField<MMSetAddress>(word, MMSetAddress::FIXED_0) == 0) {
    MM_address_ = GetField<MMSetAddress>(word, MMSetAddress::ADDRESS);
    //cout << "SET " << MM_address_ << endl;

  } else if (GetField<MMWriteIncrement>(word, MMWriteIncrement::FIXED_1) == 1) {
    uint64_t data = GetField<MMWriteIncrement>(word, MMWriteIncrement::DATA);
    state_->SetMem(bdpars::BDMemId::MM, MM_address_, {BDWord(data)});
    //cout << "WRITE INC " << MM_address_ << " : " << data << endl;;
    MM_address_++;

  } else if (GetField<MMReadIncrement>(word, MMReadIncrement::FIXED_2) == 2) {
    BDWord data = state_->GetMem(bdpars::BDMemId::MM)->at(MM_address_);
    PushMem(bdpars::BDMemId::MM, {data});
    //cout << "READ INC " << MM_address_ << " : " << data.Packed() << endl;;
    MM_address_++;

  } else { 
    assert(false);
  }
}

void BDModel::ProcessAM(uint64_t input) {
  using namespace bdpars;

  BDWord word(input);

  if (GetField<AMSetAddress>(word, AMSetAddress::FIXED_0) == 0) {
    AM_address_ = GetField<AMSetAddress>(word, AMSetAddress::ADDRESS);
    //cout << "SET " << AM_address_ << endl;

  } else if (GetField<AMReadWrite>(word, AMReadWrite::FIXED_1) == 1) {
    BDWord old_data = state_->GetMem(bdpars::BDMemId::AM)->at(AM_address_);
    PushMem(bdpars::BDMemId::AM, {old_data});

    uint64_t new_data = GetField<AMReadWrite>(word, AMReadWrite::DATA);
    state_->SetMem(bdpars::BDMemId::AM, AM_address_, {BDWord(new_data)});
    //cout << "READ/WRITE AT " << AM_address_ << " : ";
    // for (auto& it : data_fields) {
    //  cout << "(" << it.first << ":" << it.second << "), ";
    //}
    //cout << endl;

  } else if (GetField<AMIncrement>(word, AMIncrement::FIXED_2) == 2) {
      AM_address_++;
      //cout << "INC " << AM_address_ << endl;

  } else { 
    assert(false);
  }
}

void BDModel::ProcessTAT(unsigned int TAT_idx, uint64_t input) {
  using namespace bdpars;

  BDWord word(input);

  if (GetField<TATSetAddress>(word, TATSetAddress::FIXED_0) == 0) {
    TAT_address_[TAT_idx] = GetField<TATSetAddress>(word, TATSetAddress::ADDRESS);

  } else if (GetField<TATWriteIncrement>(word, TATWriteIncrement::FIXED_1) == 1) {
    uint64_t data = GetField<TATWriteIncrement>(word, TATWriteIncrement::DATA);
    if (TAT_idx == 0) {
      state_->SetMem(bdpars::BDMemId::TAT0, TAT_address_[0], {BDWord(data)});
    } else {
      state_->SetMem(bdpars::BDMemId::TAT1, TAT_address_[1], {BDWord(data)});
    }
    TAT_address_[TAT_idx]++;

  } else if (GetField<TATReadIncrement>(word, TATReadIncrement::FIXED_2) == 2) {
    BDWord data(0);
    if (TAT_idx == 0) {
      data = state_->GetMem(bdpars::BDMemId::TAT0)->at(TAT_address_[0]);
      PushMem(bdpars::BDMemId::TAT0, {data});
    } else {
      data = state_->GetMem(bdpars::BDMemId::TAT1)->at(TAT_address_[1]);
      PushMem(bdpars::BDMemId::TAT1, {data});
    }
    TAT_address_[TAT_idx]++;

  } else { 
    assert(false);
  }
}

void BDModel::ProcessPAT(const uint64_t input) {
  using namespace bdpars;

  BDWord word(input);

  if (GetField<PATWrite>(word, PATWrite::FIXED_0) == 0) {
      unsigned int addr = GetField<PATWrite>(word, PATWrite::ADDRESS);
      uint64_t data = GetField<PATWrite>(word, PATWrite::DATA);
      state_->SetMem(bdpars::BDMemId::PAT, addr, {BDWord(data)});

  } else if (GetField<PATRead>(word, PATRead::FIXED_1) == 1) {
      unsigned int addr = GetField<PATRead>(word, PATRead::ADDRESS);
      BDWord data = state_->GetMem(bdpars::BDMemId::PAT)->at(addr);
      PushMem(bdpars::BDMemId::PAT, {data});
      //cout << "PAT READ : " << addr << endl;

  } else {
    assert(false);
  }
}

void BDModel::Process(uint8_t code, const std::vector<uint64_t>& inputs) {
  for (auto& input : inputs) {
    if (bd_pars_->DnEPCodeIsBDHornEP(code)) {
      bdpars::BDHornEP leaf_id = static_cast<bdpars::BDHornEP>(code);
      ProcessBDHorn(leaf_id, input);
    }
  }
}

void BDModel::ProcessBDHorn(bdpars::BDHornEP ep, uint64_t input) {
  using namespace bdpars;

  if (bd_pars_->BDHornEPIsReg(ep)) {
    ProcessBDReg(ep, input);

  } else if (bd_pars_->BDHornEPIsInputStream(ep)) {
    ProcessBDInputStream(ep, input);

  } else if (bd_pars_->BDHornEPIsMem(ep)) {

    switch (ep) {
      case BDHornEP::PROG_AMMM:
      {
        BDWord word(input);
        if (GetField<AMEncapsulation>(word, AMEncapsulation::FIXED_0) == 0) {
          ProcessAM(GetField<AMEncapsulation>(word, AMEncapsulation::PAYLOAD));
        } else if (GetField<MMEncapsulation>(word, MMEncapsulation::FIXED_1) == 1) {
          ProcessMM(GetField<MMEncapsulation>(word, MMEncapsulation::PAYLOAD));
        } else { 
          assert(false);
        }
        break;
      }
      case BDHornEP::PROG_PAT: {
        ProcessPAT(input);
        break;
      }
      case BDHornEP::PROG_TAT0: {
        ProcessTAT(0, input);
        break;
      }
      case BDHornEP::PROG_TAT1: {
        ProcessTAT(1, input);
        break;
      }
      default: { assert(false); }
    }
  }

}

}  // bdmodel
}  // bddriver
}  // pystorm
