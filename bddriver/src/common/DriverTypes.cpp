#include "DriverTypes.h"

#include <cassert>
#include <vector>

#include "BDPars.h"
#include "binary_util.h"

namespace pystorm {
namespace bddriver {

using namespace bdpars;  // XXX should probably just type it in everywhere...

bool operator==(const AMData& lhs, const AMData& rhs) {
  // clang-format off
  return (lhs.threshold    == rhs.threshold &&
          lhs.stop         == rhs.stop      &&
          lhs.next_address == rhs.next_address);
  // clang-format on
}

bool operator==(const PATData& lhs, const PATData& rhs) {
  // clang-format off
  return (lhs.AM_address    == rhs.AM_address    &&
          lhs.MM_address_lo == rhs.MM_address_lo &&
          lhs.MM_address_hi == rhs.MM_address_hi);
  // clang-format on
}

bool operator==(const TATData& lhs, const TATData& rhs) {
  // clang-format off
  return (lhs.stop           == rhs.stop           &&
          lhs.type           == rhs.type           &&
          lhs.AM_address     == rhs.AM_address     &&
          lhs.MM_address     == rhs.MM_address     &&
          lhs.synapse_id_0   == rhs.synapse_id_0   &&
          lhs.synapse_sign_0 == rhs.synapse_sign_0 &&
          lhs.synapse_id_1   == rhs.synapse_id_1   &&
          lhs.synapse_sign_1 == rhs.synapse_sign_1 &&
          lhs.tag            == rhs.tag            &&
          lhs.global_route   == rhs.global_route);
  // clang-format on
}

bool operator==(const SynSpike& lhs, const SynSpike& rhs) {
  // clang-format off
  return (lhs.time       == rhs.time       &&
          lhs.core_id    == rhs.core_id    &&
          lhs.synapse_id == rhs.synapse_id &&
          lhs.sign       == rhs.sign);
  // clang-format on
}

bool operator==(const NrnSpike& lhs, const NrnSpike& rhs) {
  // clang-format off
  return (lhs.time       == rhs.time    &&
          lhs.core_id    == rhs.core_id &&
          lhs.neuron_id == rhs.neuron_id);
  // clang-format on
}

bool operator==(const Tag& lhs, const Tag& rhs) {
  // clang-format off
  return (lhs.time       == rhs.time       &&
          lhs.core_id    == rhs.core_id    &&
          lhs.tag        == rhs.tag        &&
          lhs.global_tag == rhs.global_tag &&
          lhs.count      == rhs.count);
  // clang-format on
}

uint64_t SignedValToSignBit(int sign) {
  uint64_t bit = static_cast<uint64_t>((-1 * sign + 1) / 2);
  assert(((bit == 0 && sign == 1) || (bit == 1 && sign == -1)) && "sign(bit) must 0(+1) or 1(-1)");
  return bit;
}

int SignBitToSignedVal(uint64_t bit) {
  int sign = -2 * static_cast<int>(bit) + 1;
  assert(((bit == 0 && sign == 1) || (bit == 1 && sign == -1)) && "sign(bit) must 0(+1) or 1(-1)");
  return sign;
}

FieldValues DataToFieldValues(const PATData& data) {
  return {{AM_ADDRESS, data.AM_address}, {MM_ADDRESS_LO, data.MM_address_lo}, {MM_ADDRESS_HI, data.MM_address_hi}};
}

FieldValues DataToFieldValues(const TATData& data) {
  // TAT is more complicated, has three possible data packings, different return type for this fn than the others

  if (data.type == 0) {  // address type
    return {{AM_ADDRESS, data.AM_address}, {MM_ADDRESS, data.MM_address}, {STOP, data.stop}};

  } else if (data.type == 1) {  // spike type
    return {{SYNAPSE_ADDRESS_0, data.synapse_id_0},
            {SYNAPSE_SIGN_0, SignedValToSignBit(data.synapse_sign_0)},  // -1/+1 -> 1/0
            {SYNAPSE_ADDRESS_1, data.synapse_id_1},
            {SYNAPSE_SIGN_1, SignedValToSignBit(data.synapse_sign_1)},  // -1/+1 -> 1/0
            {STOP, data.stop}};

  } else if (data.type == 2) {  // fanout type
    return {{TAG, data.tag}, {GLOBAL_ROUTE, data.global_route}, {STOP, data.stop}};
  } else {
    assert(false);
    return {};
  }
}

FieldValues DataToFieldValues(const AMData& data) {
  return {{ACCUMULATOR_VALUE, 0}, {THRESHOLD, data.threshold}, {STOP, data.stop}, {NEXT_ADDRESS, data.next_address}};
}

FieldValues DataToFieldValues(const MMData& data) {
  // XXX should do two's complement -> one's complement here
  return {{WEIGHT, data}};
}

FieldValues DataToFieldValues(const SynSpike& data) {
  return {{SYNAPSE_ADDRESS, data.synapse_id}, {SYNAPSE_SIGN, data.sign}};
}

FieldValues DataToFieldValues(const NrnSpike& data) { return {{NEURON_ADDRESS, data.neuron_id}}; }

FieldValues DataToFieldValues(const Tag& data) { return {{COUNT, data.count}, {TAG, data.tag}, {GLOBAL_ROUTE, data.global_tag}}; }

/// XXX these are all exactly the same, could be templated
VFieldValues DataToVFieldValues(const std::vector<PATData>& data) {
  VFieldValues output;
  for (auto& el : data) {
    output.push_back(DataToFieldValues(el));
  }
  return output;
}

VFieldValues DataToVFieldValues(const std::vector<TATData>& data) {
  VFieldValues output;
  for (auto& el : data) {
    output.push_back(DataToFieldValues(el));
  }
  return output;
}

VFieldValues DataToVFieldValues(const std::vector<AMData>& data) {
  VFieldValues output;
  for (auto& el : data) {
    output.push_back(DataToFieldValues(el));
  }
  return output;
}

VFieldValues DataToVFieldValues(const std::vector<MMData>& data) {
  VFieldValues output;
  for (auto& el : data) {
    output.push_back(DataToFieldValues(el));
  }
  return output;
}

VFieldValues DataToVFieldValues(const std::vector<SynSpike>& data) {
  VFieldValues output;
  for (auto& el : data) {
    output.push_back(DataToFieldValues(el));
  }
  return output;
}

VFieldValues DataToVFieldValues(const std::vector<NrnSpike>& data) {
  VFieldValues output;
  for (auto& el : data) {
    output.push_back(DataToFieldValues(el));
  }
  return output;
}

VFieldValues DataToVFieldValues(const std::vector<Tag>& data) {
  VFieldValues output;
  for (auto& el : data) {
    output.push_back(DataToFieldValues(el));
  }
  return output;
}

PATData FieldValuesToPATData(const FieldValues& field_values) {
  PATData data;
  data.AM_address    = FVGet(field_values, AM_ADDRESS);
  data.MM_address_lo = FVGet(field_values, MM_ADDRESS_LO);
  data.MM_address_hi = FVGet(field_values, MM_ADDRESS_HI);
  return data;
}

std::vector<PATData> FieldValuesToPATData(const VFieldValues& vfv) {
  std::vector<PATData> outputs;
  for (auto& fv : vfv) {
    outputs.push_back(FieldValuesToPATData(fv));
  }
  return outputs;
}

TATData FieldValuesToTATData(const FieldValues& field_values) {
  // clear all fields, set the ones we need later
  TATData data = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  data.stop    = FVGet(field_values, STOP);
  if (FVContains(field_values, AM_ADDRESS)) {
    data.type       = 0;
    data.AM_address = FVGet(field_values, AM_ADDRESS);
    data.MM_address = FVGet(field_values, MM_ADDRESS);

  } else if (FVContains(field_values, SYNAPSE_ADDRESS_0)) {
    data.type           = 1;
    data.synapse_id_0   = FVGet(field_values, SYNAPSE_ADDRESS_0);
    data.synapse_sign_0 = SignBitToSignedVal(FVGet(field_values, SYNAPSE_SIGN_0));
    data.synapse_id_1   = FVGet(field_values, SYNAPSE_ADDRESS_1);
    data.synapse_sign_1 = SignBitToSignedVal(FVGet(field_values, SYNAPSE_SIGN_1));

  } else if (FVContains(field_values, TAG)) {
    data.type         = 2;
    data.tag          = FVGet(field_values, TAG);
    data.global_route = FVGet(field_values, GLOBAL_ROUTE);
  }
  return data;
}

AMData FieldValuesToAMData(const FieldValues& field_values) {
  AMData data;
  data.threshold    = FVGet(field_values, THRESHOLD);
  data.stop         = FVGet(field_values, STOP);
  data.next_address = FVGet(field_values, NEXT_ADDRESS);
  // ignore value
  return data;
}

MMData FieldValuesToMMData(const FieldValues& field_values) {
  // XXX should un-convert from one's complement
  return FVGet(field_values, WEIGHT);
}

SynSpike FieldValuesToSynSpike(const FieldValues& field_values, unsigned int time, unsigned int core_id) {
  SynSpike data;
  data.time       = time;
  data.core_id    = core_id;
  data.synapse_id = FVGet(field_values, SYNAPSE_ADDRESS);
  data.sign       = SignBitToSignedVal(FVGet(field_values, SYNAPSE_SIGN));
  return data;
}

NrnSpike FieldValuesToNrnSpike(const FieldValues& field_values, unsigned int time, unsigned int core_id) {
  NrnSpike data;
  data.time      = time;
  data.core_id   = core_id;
  data.neuron_id = FVGet(field_values, NEURON_ADDRESS);
  return data;
}

Tag FieldValuesToTag(const FieldValues& field_values, unsigned int time, unsigned int core_id) {
  Tag data;
  data.time    = time;
  data.core_id = core_id;
  data.tag     = FVGet(field_values, TAG);
  data.count   = FVGet(field_values, COUNT);
  // some tags have a global route, others don't 
  if (FVContains(field_values, GLOBAL_ROUTE)) {
    data.global_tag = FVGet(field_values, GLOBAL_ROUTE);
  } else {
    data.global_tag = 0;
  }
  return data;
}

std::vector<PATData> VFieldValuesToPATData(const VFieldValues& field_values) {
  std::vector<PATData> output;
  for (auto& fv : field_values) {
    output.push_back(FieldValuesToPATData(fv));
  }
  return output;
}

std::vector<TATData> VFieldValuesToTATData(const VFieldValues& field_values) {
  std::vector<TATData> output;
  for (auto& fv : field_values) {
    output.push_back(FieldValuesToTATData(fv));
  }
  return output;
}

std::vector<AMData> VFieldValuesToAMData(const VFieldValues& field_values) {
  std::vector<AMData> output;
  for (auto& fv : field_values) {
    output.push_back(FieldValuesToAMData(fv));
  }
  return output;
}

std::vector<MMData> VFieldValuesToMMData(const VFieldValues& field_values) {
  std::vector<MMData> output;
  for (auto& fv : field_values) {
    output.push_back(FieldValuesToMMData(fv));
  }
  return output;
}

std::vector<SynSpike> VFieldValuesToSynSpike(const VFieldValues& field_values, const std::vector<unsigned int>& times,
                                             const std::vector<unsigned int>& core_ids) {
  std::vector<SynSpike> output;
  for (unsigned int i = 0; i < field_values.size(); i++) {
    output.push_back(FieldValuesToSynSpike(field_values[i], times[i], core_ids[i]));
  }
  return output;
}

std::vector<NrnSpike> VFieldValuesToNrnSpike(const VFieldValues& field_values, const std::vector<unsigned int>& times,
                                             const std::vector<unsigned int>& core_ids) {
  std::vector<NrnSpike> output;
  for (unsigned int i = 0; i < field_values.size(); i++) {
    output.push_back(FieldValuesToNrnSpike(field_values[i], times[i], core_ids[i]));
  }
  return output;
}

std::vector<Tag> VFieldValuesToTag(const VFieldValues& field_values, const std::vector<unsigned int>& times,
                                   const std::vector<unsigned int>& core_ids) {
  std::vector<Tag> output;
  for (unsigned int i = 0; i < field_values.size(); i++) {
    output.push_back(FieldValuesToTag(field_values[i], times[i], core_ids[i]));
  }
  return output;
}

////////////////////////////////
// FVV/FV utility functions

/// Exhaustively search for field id
/// FVs are always short: this is likely to be the fastest approach
/// returns nullptr if key not found
const uint64_t* FVGetPtr(const FieldValues& fv, bdpars::WordFieldId field_id) {
  for (auto& field_and_val : fv) {
    if (field_and_val.first == field_id) {
      return &field_and_val.second;
    }
  }
  return nullptr;  // we fell through without finding anything
}

uint64_t FVGet(const FieldValues& fv, bdpars::WordFieldId field_id) {
  const uint64_t* ptr = FVGetPtr(fv, field_id);
  assert(ptr != nullptr && "shouldn't be using this where you aren't guaranteed the key is present");
  return *ptr;
}

bool FVContains(const FieldValues& fv, bdpars::WordFieldId field_id) {
  const uint64_t* ptr = FVGetPtr(fv, field_id);
  return ptr != nullptr;
}

bool FVContainsWordStruct(const FieldValues& fv, const bdpars::WordStructure& word_struct) {
  // XXX this is a little sketchy because I ignore DATA/PAYLOAD/UNUSED in the word struct
  // these are all fields that get expanded, so they're often not
  // present in the word you're actually comparing in BDModel
  using namespace bdpars;

  bool is_word_struct = true;
  for (std::pair<WordFieldId, unsigned int> field : word_struct) {
    WordFieldId field_id = field.first;
    if (field_id != DATA && field_id != UNUSED && field_id != PAYLOAD) {
      is_word_struct = is_word_struct && FVContains(fv, field_id);
    }
  }
  return is_word_struct;
}

bool FVExactlyMatchesWordStruct(const FieldValues& fv, const bdpars::WordStructure& word_struct) {
  bool is_word_struct                                 = true;
  if (word_struct.size() != fv.size()) is_word_struct = false;
  if (!FVContainsWordStruct(fv, word_struct)) is_word_struct = false;
  return is_word_struct;
}

uint64_t PackWord(const bdpars::WordStructure& word_struct, const FieldValues& field_values) {
  std::vector<unsigned int> widths_as_vect;
  std::vector<uint64_t> field_values_as_vect;
  for (auto& it : word_struct) {
    bdpars::WordFieldId field_id;
    unsigned int field_width;
    std::tie(field_id, field_width) = it;

    uint64_t field_value;
    if (FVContains(field_values, field_id)) {
      field_value = FVGet(field_values, field_id);
    } else {
      field_value = bdpars::BDPars::ValueForSpecialFieldId(field_id);
    }

    widths_as_vect.push_back(field_width);
    field_values_as_vect.push_back(field_value);
  }

  return PackV64(field_values_as_vect, widths_as_vect);
}

std::vector<uint64_t> PackWords(const bdpars::WordStructure& word_struct, const VFieldValues& vfv) {
  std::vector<uint64_t> output;
  output.reserve(vfv.size());
  for (auto& fv : vfv) {
    output.push_back(PackWord(word_struct, fv));
  }
  return output;
}

FieldValues UnpackWord(const bdpars::WordStructure& word_struct, uint64_t word) {
  std::vector<unsigned int> widths_as_vect;
  for (auto& it : word_struct) {
    unsigned int field_width = it.second;
    widths_as_vect.push_back(field_width);
  }

  std::vector<uint64_t> vals = UnpackV64(word, widths_as_vect);

  FieldValues output;
  for (unsigned int i = 0; i < word_struct.size(); i++) {
    bdpars::WordFieldId field_id = word_struct.at(i).first;
    if (!bdpars::BDPars::SpecialFieldValueMatches(field_id, vals[i])) return {};
    output.push_back({field_id, vals[i]});
  }
  return output;
}

VFieldValues UnpackWords(const bdpars::WordStructure& word_struct, std::vector<uint64_t> words) {
  VFieldValues outputs;
  for (auto& word : words) {
    outputs.push_back(UnpackWord(word_struct, word));
  }
  return outputs;
}

std::pair<FieldValues, bdpars::MemWordId> UnpackMemWordNWays(
    uint64_t input, 
    std::vector<bdpars::MemWordId> words_to_try,
    const bdpars::BDPars * bd_pars) {
  bool found = false;
  bdpars::MemWordId found_id;
  FieldValues found_field_vals;
  for (auto& word_id : words_to_try) {
    FieldValues field_vals = UnpackWord(*bd_pars->Word(word_id), input);
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

}  // bddriver namespace
}  // pystorm namespace
