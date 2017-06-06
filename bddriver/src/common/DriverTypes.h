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
  int synapse_sign_0;  // -1 or +1
  unsigned int synapse_id_1;
  int synapse_sign_1;  // -1 or +1

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
// bool operator==(const MMData & lhs, const MMData & rhs); // for now, MMData is just a uint, so this exists
bool operator==(const AMData& lhs, const AMData& rhs);
bool operator==(const PATData& lhs, const PATData& rhs);
bool operator==(const TATData& lhs, const TATData& rhs);
bool operator==(const SynSpike& lhs, const SynSpike& rhs);
bool operator==(const NrnSpike& lhs, const NrnSpike& rhs);
bool operator==(const Tag& lhs, const Tag& rhs);

////////////////////////////////////////
// internal word stream def'ns

/// An unordered list of field-to-value mappings.
/// This is used against a WordStructure to pack/unpack multiple fields into/from words.
/// This allows for some separation of functionality: the only part of the
/// driver that knows anything about the structure of the words is BDPars.
/// Higher-level driver functionality only has to associate values with field ids
typedef std::vector<std::pair<bdpars::WordFieldId, uint64_t> > FieldValues;
typedef std::vector<FieldValues> VFieldValues;

////////////////////////////////
// conversion functions from user types to FieldValues

uint64_t SignedValToSignBit(int sign);
int SignBitToSignedVal(uint64_t bit);

FieldValues DataToFieldValues(const PATData& data);
FieldValues DataToFieldValues(const TATData& data);
FieldValues DataToFieldValues(const AMData& data);
FieldValues DataToFieldValues(const MMData& data);
FieldValues DataToFieldValues(const SynSpike& data);
FieldValues DataToFieldValues(const NrnSpike& data);
FieldValues DataToFieldValues(const Tag& data);

PATData FieldValuesToPATData(const FieldValues& field_values);
TATData FieldValuesToTATData(const FieldValues& field_values);
AMData FieldValuesToAMData(const FieldValues& field_values);
MMData FieldValuesToMMData(const FieldValues& field_values);
// these need extra args, times and core_ids aren't kept in FVVs ever
SynSpike FieldValuesToSynSpike(const FieldValues& field_values, unsigned int times, unsigned int core_ids);
NrnSpike FieldValuesToNrnSpike(const FieldValues& field_values, unsigned int times, unsigned int core_ids);
Tag FieldValuesToTag(const FieldValues& field_values, unsigned int times, unsigned int core_ids);

////////////////////////////////
// conversion functions from user types to VFieldValues
// just calling the above functions in a loop

VFieldValues DataToVFieldValues(const std::vector<PATData>& data);
VFieldValues DataToVFieldValues(const std::vector<TATData>& data);
VFieldValues DataToVFieldValues(const std::vector<AMData>& data);
VFieldValues DataToVFieldValues(const std::vector<MMData>& data);
VFieldValues DataToVFieldValues(const std::vector<SynSpike>& data);
VFieldValues DataToVFieldValues(const std::vector<NrnSpike>& data);
VFieldValues DataToVFieldValues(const std::vector<Tag>& data);

std::vector<PATData> VFieldValuesToPATData(const VFieldValues& field_values);
std::vector<TATData> VFieldValuesToTATData(const VFieldValues& field_values);
std::vector<AMData> VFieldValuesToAMData(const VFieldValues& field_values);
std::vector<MMData> VFieldValuesToMMData(const VFieldValues& field_values);
// these need extra args, times and core_ids aren't kept in FVVs ever
std::vector<SynSpike> VFieldValuesToSynSpike(const VFieldValues& field_values, const std::vector<unsigned int>& times,
                                             const std::vector<unsigned int>& core_ids);
std::vector<NrnSpike> VFieldValuesToNrnSpike(const VFieldValues& field_values, const std::vector<unsigned int>& times,
                                             const std::vector<unsigned int>& core_ids);
std::vector<Tag> VFieldValuesToTag(const VFieldValues& field_values, const std::vector<unsigned int>& times,
                                   const std::vector<unsigned int>& core_ids);

////////////////////////////////
// FieldValues utility functions

/// Get ptr to value of FV field with field_id, returns nullptr if value not found
const uint64_t* FVGetPtr(const FieldValues& fv, bdpars::WordFieldId field_id);

/// Get value of FV field with field_id, assertion failure if value not found
uint64_t FVGet(const FieldValues& fv, bdpars::WordFieldId field_id);

/// Returns true if FV has field with field_id
bool FVContains(const FieldValues& fv, bdpars::WordFieldId field_id);

/// Returns true if FV contains all field_ids in word_struct
bool FVContainsWordStruct(const FieldValues& fv, const bdpars::WordStructure& word_struct);

/// Returns true if FV contains all field_ids in word_struct and has no other fields
bool FVExactlyMatchesWordStruct(const FieldValues& fv, const bdpars::WordStructure& word_struct);


/// Packs FV into a uint64_t according to word_struct
uint64_t PackWord(const bdpars::WordStructure &word_struct, const FieldValues &field_values);

/// calls PackWord for a VFV, generating a vector of uint64_ts
std::vector<uint64_t> PackWords(const bdpars::WordStructure &word_struct, const VFieldValues &vfv);

/// Unpack a uint64_t into a FV according to word_struct
FieldValues UnpackWord(const bdpars::WordStructure &word_struct, uint64_t word);

/// calls UnpackWord for a vector of uint64_ts, generating a VFV
VFieldValues UnpackWords(const bdpars::WordStructure &word_struct, std::vector<uint64_t> words);

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

}  // bddriver
}  // pystorm

#endif
