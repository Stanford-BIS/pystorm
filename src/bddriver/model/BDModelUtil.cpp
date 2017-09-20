#include "BDModelUtil.h"

#include "common/BDState.h"
#include "common/binary_util.h"
#include "encoder/Encoder.h" // for bytesPerOutput (XXX should be in BDPars??)
#include "decoder/Decoder.h" // for bytesPerInput (XXX should be in BDPars??)

namespace pystorm {
namespace bddriver {
namespace bdmodel {

std::vector<uint32_t> FPGAInput(std::vector<EncOutput> inputs, const bdpars::BDPars* pars) {
  // for now, just deserialize USB bytestream

  assert(inputs.size() % Encoder::bytesPerOutput == 0);

  std::vector<uint32_t> deserialized_words;
  for (unsigned int i = 0; i < inputs.size() / Encoder::bytesPerOutput; i++) {
    uint32_t word = PackWord<FPGABYTES>(
        {{FPGABYTES::B0, inputs[Encoder::bytesPerOutput * i + 0]},
         {FPGABYTES::B1, inputs[Encoder::bytesPerOutput * i + 1]},
         {FPGABYTES::B2, inputs[Encoder::bytesPerOutput * i + 2]},
         {FPGABYTES::B3, inputs[Encoder::bytesPerOutput * i + 3]}});
    deserialized_words.push_back(word);
  }
  return deserialized_words;
}

std::vector<DecInput> FPGAOutput(std::vector<uint32_t> inputs, const bdpars::BDPars* pars) {

  std::vector<DecInput> retval;
  for (auto& input : inputs) {
    uint8_t b0 = GetField<FPGABYTES>(input, FPGABYTES::B0);
    uint8_t b1 = GetField<FPGABYTES>(input, FPGABYTES::B1);
    uint8_t b2 = GetField<FPGABYTES>(input, FPGABYTES::B2);
    uint8_t b3 = GetField<FPGABYTES>(input, FPGABYTES::B3);
    retval.push_back(b0);
    retval.push_back(b1);
    retval.push_back(b2);
    retval.push_back(b3);
  }

  assert(retval.size() % Decoder::bytesPerInput == 0);
  return retval;
}

std::pair<std::vector<uint64_t>, std::vector<uint32_t> > 
  DeserializeHorn(const std::vector<uint32_t>& inputs, bdpars::BDHornEP leaf_id, const bdpars::BDPars* bd_pars) {
  unsigned int deserialization    = bd_pars->BDHorn_serialization_.at(leaf_id);
  unsigned int deserialized_width = bd_pars->BDHorn_size_.at(leaf_id);

  return DeserializeWords<uint32_t, uint64_t>(inputs, deserialized_width, deserialization);
}

std::pair<std::unordered_map<uint8_t, std::vector<uint64_t>>, 
          std::unordered_map<uint8_t, std::vector<uint32_t>>>
    DeserializeAllCodes(const std::unordered_map<uint8_t, std::vector<uint32_t>>& inputs, const bdpars::BDPars* bd_pars) {

std::pair<std::unordered_map<uint8_t, std::vector<uint64_t>>, 
          std::unordered_map<uint8_t, std::vector<uint32_t>>> outputs;

  for (auto& it : inputs) {
    if (bd_pars->DnEPCodeIsBDHornEP(it.first)) {
      uint8_t code = it.first;
      bdpars::BDHornEP leaf_id = static_cast<bdpars::BDHornEP>(code);

      std::vector<uint64_t> deserialized;
      std::vector<uint32_t> remainder;
      std::tie(deserialized, remainder) = DeserializeHorn(inputs.at(code), leaf_id, bd_pars);

      outputs.first.at(code) = deserialized;
      outputs.second.at(code) = remainder;
    }
    // XXX do something for the FPGA codes
  }

  return outputs;
}

std::pair<std::vector<uint64_t>, unsigned int> SerializeFunnel(
    const std::vector<uint64_t>& inputs, bdpars::BDFunnelEP leaf_id, const bdpars::BDPars* bd_pars) {
  unsigned int serialization = bd_pars->BDFunnel_serialization_.at(leaf_id);
  unsigned int input_width   = bd_pars->BDFunnel_size_.at(leaf_id);

  return SerializeWords<uint64_t, uint64_t>(inputs, input_width, serialization);
}

/// pairs are (serialized words, chunk widths)
std::unordered_map<uint8_t, std::pair<std::vector<uint64_t>, unsigned int>> 
    SerializeAllCodes(const std::unordered_map<uint8_t, std::vector<uint64_t>>& inputs, const bdpars::BDPars* bd_pars) {

  std::unordered_map<uint8_t, std::pair<std::vector<uint64_t>, unsigned int>> outputs;

  for (auto& it : inputs) {
    uint8_t code = it.first;
    if (bd_pars->UpEPCodeIsBDFunnelEP(code)) {
      bdpars::BDFunnelEP leaf_id = static_cast<bdpars::BDFunnelEP>(code);
      outputs.at(code) = SerializeFunnel(inputs.at(code), leaf_id, bd_pars);
    }
    // XXX do something for FPGA inputs
  }

  return outputs;
}

}  // bdmodel
}  // bddriver
}  // pystorm
