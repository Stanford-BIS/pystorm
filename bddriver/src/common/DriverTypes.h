#ifndef DRIVERTYPES_H
#define DRIVERTYPES_H

#include <cstdint>
#include <map>

#include "BDPars.h"

namespace pystorm {
namespace bddriver {

////////////////////////////////////////
// user types

struct PATData {
  /// Contents of a single PAT memory entry
  unsigned int AM_address;
  unsigned int MM_address_lo;
  unsigned int MM_address_hi;
};

struct TATData {
  /// Contents of a single TAT memory entry. 
  ///
  /// Has three field classes depending on what type of data is stored
  // (this implementation is wasteful but simple).

  unsigned int stop;
  unsigned int type;

  // type == 0 means accumulator entry
  unsigned int AM_address;
  unsigned int MM_address;

  // type == 1 means neuron entry
  unsigned int synapse_id_0;
  int          synapse_sign_0; // -1 or +1
  unsigned int synapse_id_1;
  int          synapse_sign_1; // -1 or +1
  
  // type == 2 means fanout entry
  unsigned int tag;
  unsigned int global_route;
};

struct AMData {
  /// Contents of a single AM memory entry.
  ///
  /// (the value field is not exposed to the programmer, 
  ///  programming the AM sets this field to 0)
  unsigned int threshold;
  unsigned int stop;
  unsigned int next_address;
};

/// MMData is just a weight
unsigned int typedef MMData;

struct SynSpike {
  /// A spike going to synapse
  unsigned int time;
  unsigned int core_id;
  unsigned int synapse_id;
  int sign;
};

struct NrnSpike {
  /// A spike coming from a neuron
  unsigned int time;
  unsigned int core_id;
  unsigned int neuron_id;
};


struct Tag {
  /// A tag going to or from the datapath
  unsigned int time;
  unsigned int core_id;
  unsigned int tag;
  unsigned int count;
};

////////////////////////////////////////
// equality tests
//bool operator==(const MMData & lhs, const MMData & rhs); // for now, MMData is just a uint, so this exists
bool operator==(const AMData   & lhs, const AMData   & rhs);
bool operator==(const PATData  & lhs, const PATData  & rhs);
bool operator==(const TATData  & lhs, const TATData  & rhs);
bool operator==(const SynSpike & lhs, const SynSpike & rhs);
bool operator==(const NrnSpike & lhs, const NrnSpike & rhs);
bool operator==(const Tag      & lhs, const Tag      & rhs);

////////////////////////////////////////
// internal word stream def'ns 

// typedefs: words and word streams
typedef std::map<bdpars::WordFieldId, uint64_t> FieldValues;
typedef std::map<bdpars::WordFieldId, std::vector<uint64_t> > FieldVValues;

////////////////////////////////
// conversion functions from user types to FieldVValues

uint64_t SignedValToSignBit(int sign);

FieldVValues             DataToFieldVValues(const std::vector<PATData> & data);
std::vector<FieldValues> DataToFieldVValues(const std::vector<TATData> & data); // TAT can have mixed field types
FieldVValues             DataToFieldVValues(const std::vector<AMData> & data);
FieldVValues             DataToFieldVValues(const std::vector<MMData> & data);
FieldVValues             DataToFieldVValues(const std::vector<SynSpike> & data);
FieldVValues             DataToFieldVValues(const std::vector<NrnSpike> & data);
FieldVValues             DataToFieldVValues(const std::vector<Tag> & data);

std::vector<PATData>  FieldVValuesToPATData(const FieldVValues & field_values);
std::vector<TATData>  FieldVValuesToTATData(const std::vector<FieldValues> & field_values);
std::vector<AMData>   FieldVValuesToAMData(const FieldVValues & field_values);
std::vector<MMData>   FieldVValuesToMMData(const FieldVValues & field_values);
// these need extra args, times and core_ids aren't kept in FVVs ever
std::vector<SynSpike> FieldVValuesToSynSpike(const FieldVValues & field_values, 
                                             const std::vector<unsigned int> & times, 
                                             const std::vector<unsigned int> & core_ids);
std::vector<NrnSpike> FieldVValuesToNrnSpike(const FieldVValues & field_values, 
                                             const std::vector<unsigned int> & times, 
                                             const std::vector<unsigned int> & core_ids);
std::vector<Tag>      FieldVValuesToTag(const FieldVValues & field_values, 
                                             const std::vector<unsigned int> & times, 
                                             const std::vector<unsigned int> & core_ids);

////////////////////////////////
// FieldVValues, FieldValues utility functions

FieldVValues FVasFVV(const FieldValues & input);
std::vector<FieldValues> FVVasVFV(const FieldVValues & input);
std::vector<FieldValues> VFVVasVFV(const std::vector<FieldVValues> & input);
bool FVVKeysMatchFV(const FieldVValues & fvv, const FieldValues & fv);
void AppendToFVV(FieldVValues * fvv, const FieldValues & to_append);
std::vector<FieldVValues> VFVasVFVV(const std::vector<FieldValues> & inputs);
FieldVValues CollapseFVVs(const std::vector<std::pair<FieldVValues, bdpars::WordFieldId> > & fvvs);
FieldValues CollapseFVs(const std::vector<std::pair<FieldValues, bdpars::WordFieldId> > & fvs);

bool FVVContainsWordStruct(const FieldVValues & fvv, const bdpars::WordStructure & word_struct);
bool FVVExactlyMatchesWordStruct(const FieldVValues & fvv, const bdpars::WordStructure & word_struct);

////////////////////////////////////////
// decoder/encoder

// decoder
struct DecOutput {
  uint32_t payload;
  unsigned int core_id;
  unsigned int time_epoch;
};
typedef uint8_t DecInput;

// encoder
struct EncInput {
  unsigned int core_id;
  unsigned int leaf_id;
  uint32_t payload;
};
typedef uint8_t EncOutput;

} // bddriver
} // pystorm

#endif
