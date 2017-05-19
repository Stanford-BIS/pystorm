#include "BDModelUtil.h"

#include "common/binary_util.h"
#include "common/BDState.h"
#include "encoder/Encoder.h"

namespace pystorm {
namespace bddriver {
namespace bdmodel {

bool BDStatesMatch(const BDState * a, const BDState * b) 
{
  // XXX
}

std::vector<uint32_t> FPGAInput(std::vector<EncOutput> inputs) 
{
  // for now, just deserialize USB bytestream
  
  assert(inputs.size() % Encoder::bytesPerOutput == 0);

  std::vector<uint32_t> deserialized_words;
  for (unsigned int i = 0; i < inputs.size() / Encoder::bytesPerOutput; i++) {
    uint32_t word = PackV32(
        {inputs[i+0], inputs[i+1], inputs[i+2]},
        {8, 8, 8}
    );
    deserialized_words.push_back(word);
  }
  return deserialized_words;
}

std::vector<std::vector<uint32_t> > Horn(const std::vector<uint32_t> & inputs, const bdpars::BDPars * pars) 
{
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

    uint32_t route_val = static_cast<uint32_t>(it.first);
    uint32_t route_len = static_cast<uint32_t>(it.second);
    uint32_t payload_len = static_cast<uint32_t>(pars->Width(bdpars::BD_output)) - route_len;

    // route mask looks like 000000001111
    uint32_t route_mask = (one << route_len) - one
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
    
    unsigned int horn_id;
    uint32_t payload;
    std::tie(horn_id, payload) = DecodeFH<uint32_t, uint32_t>(
        input,
        leaf_routes,
        leaf_route_masks,
        leaf_payload_masks,
        leaf_payload_shifts
    );

    // put decoded outputs into the appropriate vectors
    outputs[horn_id].push_back(payload);
  }
}

std::pair<std::vector<uint64_t>, std::vector<uint32_t> > DeserializeHorn(const std::vector<uint32_t> & inputs, bdpars::HornLeafId leaf_id, const bdpars::BDPars * bd_pars)
{
  unsigned int deserialization = bd_pars->Serialization(leaf_id); 
  unsigned int deserialized_width = bd_pars->Width(leaf_id);

  return DeserializeWords<uint32_t, uint64_t>(inputs, deserialized_width, deserialization);
}

std::vector<std::vector<uint64_t> > DeserializeAllHornLeaves(const std::vector<std::vector<uint32_t> > & inputs, const bdpars::BDPars * bd_pars)
{
  assert(inputs.size() == static_cast<unsigned int>(bdpars::LastHornLeafId));

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
    for (auto& it : deserialized) {
      outputs[i].push_back(it);
    }
  }

  return outputs;
}

// helper used in DeencapsulateWord, not exposed in the header
unsigned int FindGoodFV(const std::vector<FieldValues> & fvs)
{
  bool already_found = false;
  unsigned int found_idx;
  for (unsigned int i = 0; i < fvs.size(); i++) {
    if (fvs[i].size() > 0) {
      assert(!already_found);
      already_found = true;
      found_idx = i;
    }
  }
  return found_idx;
}

// combination of FindGoodFV and CollapseFVs
std::pair<unsigned int, FieldValues> DeencapsulateWord(const FieldValues & current, bdpars::WordFieldId descend_into, const std::vector<const bdpars::WordStructure *> & words_to_try)
{
  using namespace bdpars;

  std::vector<FieldValues> fields;
  for (auto& word : words_to_try) {
    fields.push_back(Driver::UnpackWord(*word, current.at(descend_into)));
  }
  unsigned int good_field_idx = FindGoodFV(fields);

  FieldValues collapsed_fv = CollapseFVs({{current, descend_into}, {fields[good_field_idx], INVALID_FIELD}});
  return {good_field_idx, collapsed_fv};
}

// special case of the above, you can just pass MemWordIds
// not exposed in header since it's pretty specific
std::pair<unsigned int, FieldValues> DeencapsulateMemWord(const FieldValues & current, bdpars::WordFieldId descend_into, const std::vector<bdpars::MemWordId> & words_to_try, const bdpars::BDPars * bd_pars)
{
  using namespace bdpars;

  std::vector<const WordStructure *> words;
  for (auto& word_id : words_to_try) {
    words.push_back(bd_pars->Word(word_id));
  }

  return DeencapsulateWord(current, descend_into, words);
}

// helper function to call Deencapsulate for all the mem data words,
// not eposed in header
FieldValues ParseMemDataWord(
    FieldValues input,
    bdpars::MemId mem_id,
    bdpars::MemWordId data_word,
    const std::vector<bdpars::MemWordId> & other_words,
    const bdpars::BDPars * bd_pars
)
{
  using namespace bdpars;

  // make all_words out of the provided words, with the data_word always at the back
  std::vector<MemWordId> all_words = other_words;
  all_words.push_back(data_word);

  unsigned int match_idx;
  FieldValues fields;
  std::tie(match_idx, fields) = 
    DeencapsulateMemWord(
        input, 
        PAYLOAD,
        all_words,
        bd_pars
  );

  // unpack data, if it's a data_word
  if (match_idx == all_words.size() - 1) {

    if (mem_id == TAT0 || mem_id == TAT1) { // TAT is special, has multiple words
      unsigned int TAT_DATA_match_idx; // unused
      FieldValues TAT_DATA_fields;
      std::tie(TAT_DATA_match_idx, TAT_DATA_fields) = 
        DeencapsulateWord(
            fields,
            DATA, 
            {bd_pars->Word(mem_id, 0), bd_pars->Word(mem_id, 1), bd_pars->Word(mem_id, 2)}
      );
      return TAT_DATA_fields;

    } else {
      unsigned int DATA_match_idx; // unused
      FieldValues DATA_fields;
      std::tie(DATA_match_idx, DATA_fields) = 
        DeencapsulateWord(
            fields,
            DATA, 
            {bd_pars->Word(mem_id)}
      );
      return DATA_fields;
    }

  } else {
    return fields;
  }
}

FieldValues ParseMemDataWord(
    uint64_t input,
    bdpars::MemId mem_id,
    bdpars::MemWordId data_word,
    const std::vector<bdpars::MemWordId> & other_words,
    const bdpars::BDPars * bd_pars
)
{
  using namespace bdpars;
  return ParseMemDataWord({{PAYLOAD, input}}, mem_id, data_word, other_words, bd_pars); 
}

FieldValues ParseAMWord(FieldValues input, const bdpars::BDPars * bd_pars)
{
  using namespace bdpars;
  ParseMemDataWord(input, AM, AM_READ_WRITE, {AM_SET_ADDRESS, AM_INCREMENT}, bd_pars);
}

FieldValues ParseMMWord(FieldValues input, const bdpars::BDPars * bd_pars)
{
  using namespace bdpars;
  ParseMemDataWord(input, MM, MM_WRITE_INCREMENT, {MM_SET_ADDRESS, MM_READ_INCREMENT}, bd_pars);
}


FieldValues ParseAMMMWord(uint64_t input, const bdpars::BDPars * bd_pars)
{
  using namespace bdpars;

  unsigned int AMMM_match_idx;
  FieldValues AMMM_fields;
  // initial call to this function is weird because you still have to have/decend into a FVV
  std::tie(AMMM_match_idx, AMMM_fields) = 
    DeencapsulateMemWord(
      {{INVALID_FIELD, input}},
      INVALID_FIELD, 
      {AM_ENCAPSULATION, MM_ENCAPSULATION}, 
      bd_pars
  );
  
  // unpack AM
  if (AMMM_match_idx == 0) {
    return ParseAMWord(AMMM_fields, bd_pars);
  } else {
    return ParseMMWord(AMMM_fields, bd_pars);
  }

}

FieldValues ParsePATWord(uint64_t input, const bdpars::BDPars * bd_pars)
{
  using namespace bdpars;
  ParseMemDataWord(input, PAT, PAT_WRITE, {PAT_READ}, bd_pars);
}

FieldValues ParseTATWord(uint64_t input, const bdpars::BDPars * bd_pars)
{
  using namespace bdpars;
  ParseMemDataWord(input, TAT0, TAT_WRITE_INCREMENT, {TAT_SET_ADDRESS, TAT_READ_INCREMENT}, bd_pars);
}

// helper for ParseHornLeaf, not exposed in header
FieldValues ParseHornLeafInput(uint64_t input, bdpars::HornLeafId leaf_id, const bdpars::BDPars * bd_pars)
{
  using namespace bdpars;

  ComponentTypeId component_type = bd_pars->ComponentTypeIdFor(leaf_id);
  unsigned int component_idx = bd_pars->ComponentIdxFor(leaf_id);

  switch (component_type) {
    // reg and input are basically the same
    case REG :
    {
      RegId reg_id = static_cast<RegId>(component_idx);
      return  Driver::UnpackWord(*bd_pars->Word(reg_id), input);
    }

    // reg and input are basically the same
    case INPUT :
    {
      InputId input_id = static_cast<InputId>(component_idx);
      return Driver::UnpackWord(*bd_pars->Word(input_id), input);
    }

    // memory is complicated =(
    case MEM :
    {
      MemId mem_id = static_cast<MemId>(component_idx);

      switch (mem_id) {
        case MM : // actually this catches AM too
        {
          return ParseAMMMWord(input, bd_pars);
        }
        case PAT :
        {
          return ParsePATWord(input, bd_pars);
        }
        case TAT0 :
        {
          return ParseTATWord(input, bd_pars);
        }
        case TAT1 :
        {
          return ParseTATWord(input, bd_pars);
        }
        default :
        {
          assert(false);
          return {};
        }
      }
    }

    default :
    {
      assert(false);
      return {};
    }
  }
}

std::vector<FieldVValues> ParseHornLeaf(const std::vector<uint64_t> inputs, bdpars::HornLeafId leaf_id, const bdpars::BDPars * bd_pars)
{
  // parse each input individually
  std::vector<FieldValues> fields;
  for (auto& input : inputs) {
    fields.push_back(ParseHornLeafInput(input, leaf_id, bd_pars));
  }

  // compress into FieldVValues
  return VFVAsVFVV(fields);
}


} // bdmodel
} // bddriver
} // pystorm
