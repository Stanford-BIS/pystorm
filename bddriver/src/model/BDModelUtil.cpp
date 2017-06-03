#include "BDModelUtil.h"

#include "common/BDState.h"
#include "common/binary_util.h"
#include "encoder/Encoder.h"

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

std::vector<std::vector<uint32_t> > Horn(const std::vector<uint32_t>& inputs, const bdpars::BDPars* pars) {
  // this code is based on the Decoder's, but the fields are arranged differently

  // encoder word
  // msb <- lsb
  // [ X | payload | route ]

  std::vector<uint32_t> leaf_routes;
  std::vector<uint32_t> leaf_route_masks;
  std::vector<uint32_t> leaf_payload_masks;
  std::vector<unsigned int> leaf_payload_shifts;

  // generate routing info from pars (copy-pasted from Decoder's constructor, then adapted)
  for (const bdpars::FHRoute& it : *(pars->HornRoutes())) {
    uint32_t one = 1;

    uint32_t route_val   = static_cast<uint32_t>(it.first);
    uint32_t route_len   = static_cast<uint32_t>(it.second);
    uint32_t payload_len = static_cast<uint32_t>(pars->Width(bdpars::BD_INPUT)) - route_len;

    // route mask looks like 000000001111
    uint32_t route_mask = (one << route_len) - one;
    leaf_route_masks.push_back(route_mask);

    // payload mask looks like 111111110000
    uint32_t payload_mask = ((one << payload_len) - one) << route_len;
    leaf_payload_masks.push_back(payload_mask);

    // route looks like 000000001011
    leaf_routes.push_back(route_val);

    leaf_payload_shifts.push_back(route_len);
  }

  // one output vector per leaf
  std::vector<std::vector<uint32_t> > outputs;
  for (unsigned int i = 0; i < leaf_routes.size(); i++) {
    outputs.push_back({});
  }

  // use the routing info to decode
  for (auto& input : inputs) {
    // cout << "horn input: " << UintAsString<uint32_t>(input, 21) << endl;

    unsigned int horn_id;
    uint32_t payload;
    std::tie(horn_id, payload) =
        DecodeFH<uint32_t, uint32_t>(input, leaf_routes, leaf_route_masks, leaf_payload_masks, leaf_payload_shifts);

    // put decoded outputs into the appropriate vectors
    outputs[horn_id].push_back(payload);
  }
  return outputs;
}

std::pair<std::vector<uint64_t>, std::vector<uint32_t> > DeserializeHorn(
    const std::vector<uint32_t>& inputs, bdpars::HornLeafId leaf_id, const bdpars::BDPars* bd_pars) {
  unsigned int deserialization    = bd_pars->Serialization(leaf_id);
  unsigned int deserialized_width = bd_pars->Width(leaf_id);

  return DeserializeWords<uint32_t, uint64_t>(inputs, deserialized_width, deserialization);
}

std::vector<std::vector<uint64_t> > DeserializeAllHornLeaves(
    const std::vector<std::vector<uint32_t> >& inputs, const bdpars::BDPars* bd_pars) {
  assert(inputs.size() == static_cast<unsigned int>(bdpars::LastHornLeafId) + 1);

  std::vector<std::vector<uint64_t> > outputs;
  for (unsigned int i = 0; i < inputs.size(); i++) {
    outputs.push_back({});
  }

  for (unsigned int i = 0; i < inputs.size(); i++) {
    bdpars::HornLeafId leaf_id = static_cast<bdpars::HornLeafId>(i);

    std::vector<uint64_t> deserialized;
    std::vector<uint32_t> remainder;
    std::tie(deserialized, remainder) = DeserializeHorn(inputs[i], leaf_id, bd_pars);

    assert(remainder.size() == 0 && "this call is meant to be used on complete streams only");
    outputs[i].insert(outputs[i].end(), deserialized.begin(), deserialized.end());  // append to outputs[i]
  }

  return outputs;
}

}  // bdmodel
}  // bddriver
}  // pystorm
