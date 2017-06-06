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

std::vector<uint32_t> Funnel(const std::vector<std::pair<std::vector<uint32_t>, unsigned int> >& inputs, const bdpars::BDPars* pars) {
  // msb <- lsb
  // [ route | X | payload ]
  
  assert(inputs.size() == bdpars::LastFunnelLeafId+1);

  std::vector<uint32_t> outputs;
  
  for (unsigned int i = 0; i < inputs.size(); i++) {
    uint32_t route_val;
    unsigned int route_len;
    std::tie(route_val, route_len) = pars->FunnelRoute(i);
    unsigned int payload_chunk_width = inputs.at(i).second;
    unsigned int word_width = pars->Width(bdpars::BD_OUTPUT);
    unsigned int unused_width = word_width - payload_chunk_width - route_len;

    std::vector<unsigned int> field_widths = {payload_chunk_width, unused_width, route_len};

    for (uint32_t input : inputs.at(i).first) {
      std::vector<uint32_t> field_vals = {input, 0, route_val};
      uint32_t new_word = PackV32(field_vals, field_widths);
      outputs.push_back(new_word);
    }
  }
  return outputs;
}

std::pair<std::vector<uint64_t>, std::vector<uint32_t> > DeserializeHorn(
    const std::vector<uint32_t>& inputs, bdpars::HornLeafId leaf_id, const bdpars::BDPars* bd_pars) {
  unsigned int deserialization    = bd_pars->Serialization(leaf_id);
  unsigned int deserialized_width = bd_pars->Width(leaf_id);

  return DeserializeWords<uint32_t, uint64_t>(inputs, deserialized_width, deserialization);
}

std::pair<std::vector<std::vector<uint64_t> >, std::vector<std::vector<uint32_t> > > DeserializeAllHornLeaves(
    const std::vector<std::vector<uint32_t> >& inputs, const bdpars::BDPars* bd_pars) {
  assert(inputs.size() == static_cast<unsigned int>(bdpars::LastHornLeafId) + 1);

  std::pair<std::vector<std::vector<uint64_t> >, std::vector<std::vector<uint32_t> > > outputs;
  for (unsigned int i = 0; i < inputs.size(); i++) {
    //cout << "input to D.A.H.L leaf " << i << " size was: " << inputs[i].size() << endl;
    bdpars::HornLeafId leaf_id = static_cast<bdpars::HornLeafId>(i);

    std::vector<uint64_t> deserialized;
    std::vector<uint32_t> remainder;
    std::tie(deserialized, remainder) = DeserializeHorn(inputs[i], leaf_id, bd_pars);

    outputs.first.push_back(deserialized);
    outputs.second.push_back(remainder);
  }

  return outputs;
}

std::pair<std::vector<uint32_t>, unsigned int> SerializeFunnel(
    const std::vector<uint64_t>& inputs, bdpars::FunnelLeafId leaf_id, const bdpars::BDPars* bd_pars) {
  unsigned int serialization = bd_pars->Serialization(leaf_id);
  unsigned int input_width   = bd_pars->Width(leaf_id);

  return SerializeWords<uint64_t, uint32_t>(inputs, input_width, serialization);
}

/// pairs are (serialized words, chunk widths)
std::vector<std::pair<std::vector<uint32_t>, unsigned int> > SerializeAllFunnelLeaves(
    const std::vector<std::vector<uint64_t> >& inputs, const bdpars::BDPars* bd_pars) {

  assert(inputs.size() == static_cast<unsigned int>(bdpars::LastFunnelLeafId) + 1);

  std::vector<std::pair<std::vector<uint32_t>, unsigned int> > outputs;

  for (unsigned int i = 0; i < inputs.size(); i++) {
    bdpars::FunnelLeafId leaf_id = static_cast<bdpars::FunnelLeafId>(i);
    outputs.push_back(SerializeFunnel(inputs[i], leaf_id, bd_pars));
  }

  return outputs;
}

}  // bdmodel
}  // bddriver
}  // pystorm
