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
  FieldVValues retval = {{AM_address, {}}, {MM_address_lo, {}}, {MM_address_hi, {}}};
  for (auto& it : data) {
    retval[AM_address].push_back(it.AM_address);
    retval[MM_address_lo].push_back(it.MM_address_lo);
    retval[MM_address_hi].push_back(it.MM_address_hi);
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
        {{AM_address, it.AM_address}, 
         {MM_address, it.MM_address},
         {stop,       it.stop}};

    } else if (it.type == 1) { // spike type
      field_vals = 
        {{synapse_address_0, it.synapse_address_0},
         {synapse_sign_0,    SignedValToSignBit(it.synapse_sign_0)}, // -1/+1 -> 1/0
         {synapse_address_1, it.synapse_address_1},
         {synapse_sign_1,    SignedValToSignBit(it.synapse_sign_1)}, // -1/+1 -> 1/0
         {stop,              it.stop}};

    } else if (it.type == 2) { // fanout type
        field_vals = 
          {{tag,          it.tag}, 
           {global_route, it.global_route},
           {stop,         it.stop}};
    }

    retval.push_back(field_vals);
  }

  return retval;
}

FieldVValues DataToFieldVValues(const std::vector<AMData> & data)
{
  FieldVValues retval = {{accumulator_value, {}}, {threshold, {}}, {stop, {}}, {next_address, {}}};
  for (auto& it : data) {
    retval[accumulator_value].push_back(0);
    retval[threshold].push_back(it.threshold);
    retval[stop].push_back(it.stop);
    retval[next_address].push_back(it.next_address);
  }
  return retval;
}

FieldVValues DataToFieldVValues(const std::vector<MMData> & data)
{
  // XXX should do two's complement -> one's complement here
  FieldVValues retval = {{weight, {}}};
  for (auto& it : data) {
    retval[weight].push_back(it);
  }
  return retval;
}

std::vector<PATData> FieldVValuesToPATData(const FieldVValues & field_values) 
{
  std::vector<PATData> retval;
  unsigned int num_el = field_values.begin()->second.size();
  for(unsigned int i = 0; i < num_el; i++) {
    PATData to_push;
    to_push.AM_address = field_values.at(AM_address).at(i);
    to_push.MM_address_lo = field_values.at(MM_address_lo).at(i);
    to_push.MM_address_hi = field_values.at(MM_address_hi).at(i);
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
    if (field_values.at(i).count(AM_address) > 0) {
      to_push.type = 0;
      to_push.AM_address = field_values.at(i).at(AM_address);
      to_push.MM_address = field_values.at(i).at(MM_address);
    } else if (field_values.at(i).count(synapse_address_0) > 0) {
      to_push.type = 1;
      to_push.synapse_address_0 = field_values.at(i).at(synapse_address_0);
      to_push.synapse_sign_0 = field_values.at(i).at(synapse_sign_0);
      to_push.synapse_address_1 = field_values.at(i).at(synapse_address_1);
      to_push.synapse_sign_1 = field_values.at(i).at(synapse_sign_1);
    } else if (field_values[i].count(tag) > 0) {
      to_push.type = 2;
      to_push.tag = field_values.at(i).at(tag);
      to_push.global_route = field_values.at(i).at(global_route);
    }
    to_push.stop = field_values.at(i).at(stop);

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
    to_push.threshold = field_values.at(threshold).at(i);
    to_push.stop = field_values.at(stop).at(i);
    to_push.next_address = field_values.at(next_address).at(i);
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
    MMData to_push = field_values.at(weight).at(i);
    retval.push_back(to_push);
  }
  return retval;
}

} // bddriver namespace
} // pystorm namespace
