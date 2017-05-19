#include "DriverTypes.h"

#include <cassert>
#include <vector>

#include "BDPars.h"


namespace pystorm {
namespace bddriver {

using namespace bdpars; // XXX should probably just type it in everywhere...

uint64_t SignedValToSignBit(int sign) 
{
  assert((sign == 1 || sign == -1) && "sign must be +1 or -1");
  return static_cast<uint64_t>((-1 * sign + 1) / 2);
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
        {{SYNAPSE_ADDRESS_0, it.synapse_address_0},
         {SYNAPSE_SIGN_0,    SignedValToSignBit(it.synapse_sign_0)}, // -1/+1 -> 1/0
         {SYNAPSE_ADDRESS_1, it.synapse_address_1},
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
    TATData to_push;
    if (field_values.at(i).count(AM_ADDRESS) > 0) {
      to_push.type = 0;
      to_push.AM_address = field_values.at(i).at(AM_ADDRESS);
      to_push.MM_address = field_values.at(i).at(MM_ADDRESS);
    } else if (field_values.at(i).count(SYNAPSE_ADDRESS_0) > 0) {
      to_push.type = 1;
      to_push.synapse_address_0 = field_values.at(i).at(SYNAPSE_ADDRESS_0);
      to_push.synapse_sign_0 = field_values.at(i).at(SYNAPSE_SIGN_0);
      to_push.synapse_address_1 = field_values.at(i).at(SYNAPSE_ADDRESS_1);
      to_push.synapse_sign_1 = field_values.at(i).at(SYNAPSE_SIGN_1);
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

////////////////////////////////
// FVV/FV utility functions

FieldVValues FieldValuesAsFieldVValues(const FieldValues & input) 
{
  FieldVValues output;
  for (auto& kv : input) {
    output[kv.first] = {kv.second};
  }
  return output;
}

std::vector<FieldValues> FieldVValuesAsVFieldValues(const FieldVValues & input)
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

std::vector<FieldVValues> VFVAsVFVV(const std::vector<FieldValues> & inputs)
{
  std::vector<FieldVValues> vfvv;
  for (auto& fv : inputs) {
    if (vfvv.size() == 0) {
      vfvv.push_back(FieldValuesAsFieldVValues(fv));
    } else {
      if (FVVKeysMatchFV(vfvv.back(), fv)) {
        AppendToFVV(&vfvv.back(), fv);
      } else {
        vfvv.push_back(FieldValuesAsFieldVValues(fv));
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

    fvvs.push_back({FieldValuesAsFieldVValues(fv), field_id});
  }

  return FieldVValuesAsVFieldValues(CollapseFVVs(fvvs))[0];
}


} // bddriver namespace
} // pystorm namespace
