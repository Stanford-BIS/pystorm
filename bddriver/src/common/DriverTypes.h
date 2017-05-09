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
  unsigned int synapse_address_0;
  int          synapse_sign_0; // -1 or +1
  unsigned int synapse_address_1;
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
int typedef MMData;

struct Spike {
  /// A spike going to or from a neuron
  unsigned int time;
  unsigned int core_id;
  unsigned int neuron_id;
  int sign;
};

struct Tag {
  /// A tag going to or from the datapath
  unsigned int time;
  unsigned int core_id;
  unsigned int tag_id;
  unsigned int count;
};

////////////////////////////////////////
// internal word stream def'ns 

// typedefs: words and word streams
typedef std::map<WordFieldId, uint64_t> FieldValues;
typedef std::map<WordFieldId, std::vector<uint64_t> > FieldVValues;

////////////////////////////////
// conversion functions from user types to FieldVValues

uint64_t SignedValToSignBit(int sign);

FieldVValues             DataToFieldVValues(const std::vector<PATData> & data);
std::vector<FieldValues> DataToFieldVValues(const std::vector<TATData> & data); // TAT can have mixed field types
FieldVValues             DataToFieldVValues(const std::vector<AMData> & data);
FieldVValues             DataToFieldVValues(const std::vector<MMData> & data);

std::vector<PATData> FieldVValuesToPATData(const FieldVValues & field_values);
std::vector<TATData> FieldVValuesToTATData(const std::vector<FieldValues> & field_values);
std::vector<AMData>  FieldVValuesToAMData(const FieldVValues & field_values);
std::vector<MMData>  FieldVValuesToMMData(const FieldVValues & field_values);

////////////////////////////////////////
// decoder/encoder

// decoder
struct DecOutput {
  uint32_t payload;
  unsigned int core_id;
  unsigned int time_epoch;
};
typedef uint64_t DecInput;

// encoder
struct EncInput {
  unsigned int core_id;
  unsigned int leaf_id;
  uint32_t payload;
};
typedef uint32_t EncOutput;

} // bddriver
} // pystorm

#endif
