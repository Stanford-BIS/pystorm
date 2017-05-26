#include "DriverTypes.h"

#include <cassert>
#include <vector>

#include "BDPars.h"


namespace pystorm {
namespace bddriver {

using namespace bdpars; // XXX should probably just type it in everywhere...

bool operator==(const AMData   & lhs, const AMData   & rhs)
{
  return (lhs.threshold    == rhs.threshold &&
          lhs.stop         == rhs.stop      &&
          lhs.next_address == rhs.next_address);
}

bool operator==(const PATData  & lhs, const PATData  & rhs)
{
  return (lhs.AM_address    == rhs.AM_address    &&
          lhs.MM_address_lo == rhs.MM_address_lo &&
          lhs.MM_address_hi == rhs.MM_address_hi);
}

bool operator==(const TATData  & lhs, const TATData  & rhs)
{
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
}

bool operator==(const SynSpike & lhs, const SynSpike & rhs)
{
  return (lhs.time       == rhs.time       &&
          lhs.core_id    == rhs.core_id    &&
          lhs.synapse_id == rhs.synapse_id &&
          lhs.sign       == rhs.sign);
}

bool operator==(const NrnSpike & lhs, const NrnSpike & rhs)
{
  return (lhs.time       == rhs.time    &&
          lhs.core_id    == rhs.core_id &&
          lhs.neuron_id == rhs.neuron_id);
}

bool operator==(const Tag      & lhs, const Tag      & rhs)
{
  return (lhs.time    == rhs.time    &&
          lhs.core_id == rhs.core_id &&
          lhs.tag     == rhs.tag     &&
          lhs.count   == rhs.count);
}

uint64_t SignedValToSignBit(int sign) 
{
  uint64_t bit = static_cast<uint64_t>((-1 * sign + 1) / 2);
  assert((bit == 0 && sign == 1 || bit == 1 && sign == -1) && "sign(bit) must 0(+1) or 1(-1)");
  return bit;
}

int SignBitToSignedVal(uint64_t bit) 
{
  int sign = -2*static_cast<int>(bit) + 1;
  assert((bit == 0 && sign == 1 || bit == 1 && sign == -1) && "sign(bit) must 0(+1) or 1(-1)");
  return sign;
}

FieldVValues DataToFieldVValues(const std::vector<PATData> & data)
{
  FieldVValues retval = {{AM_ADDRESS, {}}, {MM_ADDRESS_LO, {}}, {MM_ADDRESS_HI, {}}};
  for (auto& it : data) {
    retval[AM_ADDRESS].push_back(it.AM_address);
    retval[MM_ADDRESS_LO].push_back(it.MM_address_lo);
    retval[MM_ADDRESS_HI].push_back(it.MM_address_hi);
  }
  return retval;
}

std::vector<FieldValues> DataToFieldVValues(const std::vector<TATData> & data)
{
  // TAT is super complicated, has three possible data packings, different return type for this fn than the others
  std::vector<FieldValues> retval;

  for (auto& it : data) {

    FieldValues field_vals;
    if (it.type == 0) { // address type
      field_vals = 
        {{AM_ADDRESS, it.AM_address}, 
         {MM_ADDRESS, it.MM_address},
         {STOP,       it.stop}};

    } else if (it.type == 1) { // spike type
      field_vals = 
        {{SYNAPSE_ADDRESS_0, it.synapse_id_0},
         {SYNAPSE_SIGN_0,    SignedValToSignBit(it.synapse_sign_0)}, // -1/+1 -> 1/0
         {SYNAPSE_ADDRESS_1, it.synapse_id_1},
         {SYNAPSE_SIGN_1,    SignedValToSignBit(it.synapse_sign_1)}, // -1/+1 -> 1/0
         {STOP,              it.stop}};

    } else if (it.type == 2) { // fanout type
        field_vals = 
          {{TAG,          it.tag}, 
           {GLOBAL_ROUTE, it.global_route},
           {STOP,         it.stop}};
    }

    retval.push_back(field_vals);
  }

  return retval;
}

FieldVValues DataToFieldVValues(const std::vector<AMData> & data)
{
  FieldVValues retval = {{ACCUMULATOR_VALUE, {}}, {THRESHOLD, {}}, {STOP, {}}, {NEXT_ADDRESS, {}}};
  for (auto& it : data) {
    retval[ACCUMULATOR_VALUE].push_back(0);
    retval[THRESHOLD].push_back(it.threshold);
    retval[STOP].push_back(it.stop);
    retval[NEXT_ADDRESS].push_back(it.next_address);
  }
  return retval;
}

FieldVValues DataToFieldVValues(const std::vector<MMData> & data)
{
  // XXX should do two's complement -> one's complement here
  FieldVValues retval = {{WEIGHT, {}}};
  for (auto& it : data) {
    retval[WEIGHT].push_back(it);
  }
  return retval;
}

FieldVValues DataToFieldVValues(const std::vector<SynSpike> & data)
{
  FieldVValues retval = {{SYNAPSE_SIGN, {}}, {SYNAPSE_ADDRESS, {}}};
  for (auto& it : data) {
    retval[SYNAPSE_ADDRESS].push_back(it.synapse_id);
    retval[SYNAPSE_SIGN].push_back(SignedValToSignBit(it.sign));
  }
  return retval;
}

FieldVValues DataToFieldVValues(const std::vector<NrnSpike> & data)
{
  FieldVValues retval = {{NEURON_ADDRESS, {}}};
  for (auto& it : data) {
    retval[NEURON_ADDRESS].push_back(it.neuron_id);
  }
  return retval;
}

FieldVValues DataToFieldVValues(const std::vector<Tag> & data)
{
  FieldVValues retval = {{COUNT, {}}, {TAG, {}}};
  for (auto& it : data) {
    retval[COUNT].push_back(it.count);
    retval[TAG].push_back(it.tag);
  }
  return retval;
}

std::vector<PATData> FieldVValuesToPATData(const FieldVValues & field_values) 
{
  std::vector<PATData> retval;
  unsigned int num_el = field_values.begin()->second.size();
  for(unsigned int i = 0; i < num_el; i++) {
    PATData to_push;
    to_push.AM_address = field_values.at(AM_ADDRESS).at(i);
    to_push.MM_address_lo = field_values.at(MM_ADDRESS_LO).at(i);
    to_push.MM_address_hi = field_values.at(MM_ADDRESS_HI).at(i);
    retval.push_back(to_push);
  }
  return retval;
}

std::vector<TATData> FieldVValuesToTATData(const std::vector<FieldValues> & field_values) 
{
  std::vector<TATData> retval;
  unsigned int num_el = field_values.size();
  for(unsigned int i = 0; i < num_el; i++) {
    // clear all fields, set the ones we need later
    TATData to_push = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    if (field_values.at(i).count(AM_ADDRESS) > 0) {
      to_push.type = 0;
      to_push.AM_address = field_values.at(i).at(AM_ADDRESS);
      to_push.MM_address = field_values.at(i).at(MM_ADDRESS);
    } else if (field_values.at(i).count(SYNAPSE_ADDRESS_0) > 0) {
      to_push.type = 1;
      to_push.synapse_id_0 = field_values.at(i).at(SYNAPSE_ADDRESS_0);
      to_push.synapse_sign_0 = SignBitToSignedVal(field_values.at(i).at(SYNAPSE_SIGN_0));
      to_push.synapse_id_1 = field_values.at(i).at(SYNAPSE_ADDRESS_1);
      to_push.synapse_sign_1 = SignBitToSignedVal(field_values.at(i).at(SYNAPSE_SIGN_1));
    } else if (field_values[i].count(TAG) > 0) {
      to_push.type = 2;
      to_push.tag = field_values.at(i).at(TAG);
      to_push.global_route = field_values.at(i).at(GLOBAL_ROUTE);
    }
    to_push.stop = field_values.at(i).at(STOP);

    retval.push_back(to_push);
  }
  return retval;
}

std::vector<AMData> FieldVValuesToAMData(const FieldVValues & field_values)
{
  std::vector<AMData> retval;
  unsigned int num_el = field_values.begin()->second.size();
  for(unsigned int i = 0; i < num_el; i++) {
    AMData to_push;
    to_push.threshold = field_values.at(THRESHOLD).at(i);
    to_push.stop = field_values.at(STOP).at(i);
    to_push.next_address = field_values.at(NEXT_ADDRESS).at(i);
    // ignore value
    retval.push_back(to_push);
  }
  return retval;
}

std::vector<MMData> FieldVValuesToMMData(const FieldVValues & field_values)
{
  std::vector<MMData> retval;
  unsigned int num_el = field_values.begin()->second.size();
  for(unsigned int i = 0; i < num_el; i++) {
    MMData to_push = field_values.at(WEIGHT).at(i);
    retval.push_back(to_push);
  }
  return retval;
}

std::vector<SynSpike> FieldVValuesToSynSpike(const FieldVValues & field_values, const std::vector<unsigned int> & times, const std::vector<unsigned int> & core_ids)
{
  std::vector<SynSpike> retval;
  unsigned int num_el = field_values.begin()->second.size();
  for(unsigned int i = 0; i < num_el; i++) {
    SynSpike to_push;
    to_push.time = times[i];
    to_push.core_id = core_ids[i];
    to_push.synapse_id = field_values.at(SYNAPSE_ADDRESS).at(i);
    to_push.sign = SignBitToSignedVal(field_values.at(SYNAPSE_SIGN).at(i));
    // ignore value
    retval.push_back(to_push);
  }
  return retval;
}

std::vector<NrnSpike> FieldVValuesToNrnSpike(const FieldVValues & field_values, const std::vector<unsigned int> & times, const std::vector<unsigned int> & core_ids)
{
  std::vector<NrnSpike> retval;
  unsigned int num_el = field_values.begin()->second.size();
  for(unsigned int i = 0; i < num_el; i++) {
    NrnSpike to_push;
    to_push.time = times[i];
    to_push.core_id = core_ids[i];
    to_push.neuron_id = field_values.at(NEURON_ADDRESS).at(i);
    // ignore value
    retval.push_back(to_push);
  }
  return retval;
}

std::vector<Tag> FieldVValuesToTag(const FieldVValues & field_values, const std::vector<unsigned int> & times, const std::vector<unsigned int> & core_ids)
{
  std::vector<Tag> retval;
  unsigned int num_el = field_values.begin()->second.size();
  for(unsigned int i = 0; i < num_el; i++) {
    Tag to_push;
    to_push.time = times[i];
    to_push.core_id = core_ids[i];
    to_push.tag = field_values.at(TAG).at(i);
    to_push.count = field_values.at(COUNT).at(i);
    // ignore value
    retval.push_back(to_push);
  }
  return retval;
}

////////////////////////////////
// FVV/FV utility functions

FieldVValues FVasFVV(const FieldValues & input) 
{
  FieldVValues output;
  for (auto& kv : input) {
    output[kv.first] = {kv.second};
  }
  return output;
}

std::vector<FieldValues> FVVasVFV(const FieldVValues & input)
{
  std::vector<FieldValues> output;
  for (unsigned int i = 0; i < input.begin()->second.size(); i++) {
    FieldValues fv;
    for (auto& kv : input) {
      fv[kv.first] = kv.second[i];
    }
    output.push_back(fv);
  }
  return output;
}

std::vector<FieldValues> VFVVasVFV(const std::vector<FieldVValues> & inputs)
{
  std::vector<FieldValues> output;
  for (auto& input : inputs) {
    std::vector<FieldValues> this_input_as_VFV = FVVasVFV(input);
    for (auto& vfv : this_input_as_VFV) {
      output.push_back(vfv);
    }
  }
  return output;
}

bool FVVKeysMatchFV(const FieldVValues & fvv, const FieldValues & fv) 
{
  bool keys_match = true;
  for (auto & kv : fvv) {
    keys_match = keys_match && (fv.count(kv.first) > 0);
  }
  for (auto & kv : fv) {
    keys_match = keys_match && (fvv.count(kv.first) > 0);
  }
  return keys_match;
}

void AppendToFVV(FieldVValues * fvv, const FieldValues & to_append)
{
  assert(FVVKeysMatchFV(*fvv, to_append));
  for (auto& kv : *fvv) {
    fvv->at(kv.first).push_back(to_append.at(kv.first));
  }
}

std::vector<FieldVValues> VFVasVFVV(const std::vector<FieldValues> & inputs)
{
  std::vector<FieldVValues> vfvv;
  for (auto& fv : inputs) {
    if (vfvv.size() == 0) {
      vfvv.push_back(FVasFVV(fv));
    } else {
      if (FVVKeysMatchFV(vfvv.back(), fv)) {
        AppendToFVV(&vfvv.back(), fv);
      } else {
        vfvv.push_back(FVasFVV(fv));
      }
    }
  }
  return vfvv;
}

// packs a bunch of FVVs into one
FieldVValues CollapseFVVs(const std::vector<std::pair<FieldVValues, bdpars::WordFieldId> > & fvvs)
{
  // XXX for performance, this should use swaps
  // right now, this isn't being used in a way that needs performance
  using namespace bdpars; 

  FieldVValues collapsed_fvv;

  // iterate through fvv, field_id pairs
  for (auto& fvv_field : fvvs) {
    FieldVValues fvv;
    WordFieldId field_id_to_collapse_into;
    std::tie(fvv, field_id_to_collapse_into) = fvv_field;

    // iterate through fields within that fvv
    for (auto& field_vect : fvv) {
      WordFieldId field_id;
      std::vector<uint64_t> values;
      std::tie(field_id, values) = field_vect;

      // if it's not the field we're collapsing (whose values are in the next fvv), put it in the output
      if (field_id != field_id_to_collapse_into) {
        collapsed_fvv[field_id] = values;
      }
    }
  }

  // sanity check
  for (auto& it : collapsed_fvv) {
    unsigned int first_size = collapsed_fvv.begin()->second.size();
    assert(it.second.size() == first_size);
  }

  return collapsed_fvv;
}

// like CollapseFVVs, but for a FV instead
FieldValues CollapseFVs(const std::vector<std::pair<FieldValues, bdpars::WordFieldId> > & fvs) 
{
  using namespace bdpars;

  std::vector<std::pair<FieldVValues, WordFieldId> > fvvs;
  for (auto& fv_and_field : fvs) {
    FieldValues fv = fv_and_field.first; 
    WordFieldId field_id = fv_and_field.second;

    fvvs.push_back({FVasFVV(fv), field_id});
  }

  return FVVasVFV(CollapseFVVs(fvvs))[0];
}

bool FVVContainsWordStruct(const FieldVValues & fvv, const bdpars::WordStructure & word_struct)
{
  bool is_word_struct = true;
  for (std::pair<bdpars::WordFieldId, unsigned int> field : word_struct) {
    if (fvv.count(field.first) == 0) {
      is_word_struct = false;
    }
  }
  return is_word_struct;
}

bool FVVExactlyMatchesWordStruct(const FieldVValues & fvv, const bdpars::WordStructure & word_struct)
{
  bool is_word_struct = true;
  if (word_struct.size() != fvv.size()) is_word_struct = false;
  if (!FVVExactlyMatchesWordStruct(fvv, word_struct)) is_word_struct = false;
  return is_word_struct;
}

} // bddriver namespace
} // pystorm namespace
