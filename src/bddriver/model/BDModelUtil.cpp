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
    uint32_t word = PackV32(
        {inputs[Encoder::bytesPerOutput * i + 0],
         inputs[Encoder::bytesPerOutput * i + 1],
         inputs[Encoder::bytesPerOutput * i + 2]},
        {8, 8, 8});
    deserialized_words.push_back(word);
  }
  return deserialized_words;
}

std::vector<DecInput> FPGAOutput(std::vector<uint32_t> inputs, const bdpars::BDPars* pars) {

  std::vector<unsigned int> byte_widths;
  for (unsigned int i = 0; i < Decoder::bytesPerInput; i++) {
    byte_widths.push_back(8);
  }

  std::vector<DecInput> retval;
  for (auto& input : inputs) {
    std::vector<uint8_t> bytes = UnpackV<uint32_t, uint8_t>(input, byte_widths);
    retval.insert(retval.end(), bytes.begin(), bytes.end());
  }

  assert(retval.size() % Decoder::bytesPerInput == 0);
  return retval;
}

std::array<std::vector<uint32_t>, bdpars::BDEndPointIdCount> Horn(const std::vector<uint32_t>& inputs, const bdpars::BDPars* pars) {
  // Encoder word
  // msb <- lsb
  // [ leaf_id | payload ]

  // one output vector per leaf
  std::array<std::vector<uint32_t>, bdpars::BDEndPointIdCount> outputs;

  // use the routing info to decode
  for (auto& input : inputs) {
    // cout << "horn input: " << UintAsString<uint32_t>(input, 21) << endl;

    unsigned int horn_id;
    uint32_t payload;

    horn_id = (input >> 24) & 0xFF;
    payload = input & 0x00FFFFFF;

    // put decoded outputs into the appropriate vectors
    outputs[horn_id].push_back(payload);
  }
  return outputs;
}

std::vector<uint32_t> Funnel(const std::array<std::pair<std::vector<uint32_t>, unsigned int>, bdpars::BDStartPointIdCount>& inputs, const bdpars::BDPars* pars) {
  // msb <- lsb
  // [ leaf_id | payload ]
  
  std::vector<uint32_t> outputs;
  
  for (uint8_t leaf_id = 0; leaf_id < inputs.size(); leaf_id++) {
    for (uint32_t input : inputs.at(leaf_id).first) {
      uint32_t new_word = input | (leaf_id << 24);
      outputs.push_back(new_word);
    }
  }
  return outputs;
}

std::pair<std::vector<uint32_t>, std::vector<uint32_t> > DeserializeHorn(
    const std::vector<uint32_t>& inputs, bdpars::BDEndPointId leaf_id, const bdpars::BDPars* bd_pars) {
  unsigned int deserialization    = bd_pars->Serialization(leaf_id);
  unsigned int deserialized_width = bd_pars->Width(leaf_id);

  return DeserializeWords<uint32_t, uint32_t>(inputs, deserialized_width, deserialization);
}

std::pair<std::array<std::vector<uint32_t>, bdpars::BDEndPointIdCount>, 
          std::array<std::vector<uint32_t>, bdpars::BDEndPointIdCount > > 
    DeserializeAllHornLeaves(const std::array<std::vector<uint32_t>, bdpars::BDEndPointIdCount>& inputs, const bdpars::BDPars* bd_pars) {

  std::pair<std::array<std::vector<uint32_t>, bdpars::BDEndPointIdCount>, 
            std::array<std::vector<uint32_t>, bdpars::BDEndPointIdCount > > outputs;

  for (unsigned int i = 0; i < inputs.size(); i++) {
    bdpars::BDEndPointId leaf_id = static_cast<bdpars::BDEndPointId>(i);

    std::vector<uint32_t> deserialized;
    std::vector<uint32_t> remainder;
    std::tie(deserialized, remainder) = DeserializeHorn(inputs[i], leaf_id, bd_pars);

    outputs.first.at(i) = deserialized;
    outputs.second.at(i) = remainder;
  }

  return outputs;
}

std::pair<std::vector<uint32_t>, unsigned int> SerializeFunnel(
    const std::vector<uint32_t>& inputs, bdpars::BDStartPointId leaf_id, const bdpars::BDPars* bd_pars) {
  unsigned int serialization = bd_pars->Serialization(leaf_id);
  unsigned int input_width   = bd_pars->Width(leaf_id);

  return SerializeWords<uint32_t, uint32_t>(inputs, input_width, serialization);
}

/// pairs are (serialized words, chunk widths)
std::array<std::pair<std::vector<uint32_t>, unsigned int>, bdpars::BDStartPointIdCount> 
    SerializeAllFunnelLeaves(const std::array<std::vector<uint32_t>, bdpars::BDStartPointIdCount>& inputs, const bdpars::BDPars* bd_pars) {

  std::array<std::pair<std::vector<uint32_t>, unsigned int>, bdpars::BDStartPointIdCount> outputs;

  for (unsigned int i = 0; i < inputs.size(); i++) {
    bdpars::BDStartPointId leaf_id = static_cast<bdpars::BDStartPointId>(i);
    outputs.at(i) = SerializeFunnel(inputs[i], leaf_id, bd_pars);
  }

  return outputs;
}

}  // bdmodel
}  // bddriver
}  // pystorm
