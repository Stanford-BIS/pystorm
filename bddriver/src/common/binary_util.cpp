#include <assert.h>
#include <iostream>

#include <cstdint>
#include <limits>
#include <vector>
#include <string>
#include <utility>
#include <tuple>

namespace pystorm {
namespace bddriver {

using std::cout;
using std::endl;

template <class TIN, class TOUT>
inline TOUT Pack(const TIN * vals, const unsigned int * widths, unsigned int num_fields)
{
  TOUT value = 0;
  unsigned int total_width = 0;

  for (unsigned int idx = 0; idx < num_fields; idx++) {

    TIN field_val = vals[idx];
    TIN field_width = widths[idx];

    assert(field_val < std::numeric_limits<TIN>::max());
    assert(field_val < (static_cast<TIN>(1) << field_width) && "packed element value exceeds value allowed by packed element width");

    value += static_cast<TOUT>(field_val) << static_cast<TOUT>(total_width);

    total_width += field_width;
    assert(total_width <= 64 && "total width of packed elements exceeds 64");
  }
  assert(value < std::numeric_limits<TOUT>::max());
  return value;
}

template <class TIN, class TOUT> 
inline TOUT PackV(const std::vector<TIN> & vals, const std::vector<unsigned int> & widths)
{
  assert(vals.size() == widths.size() && "size of values and widths does not match");
  return Pack<TIN, TOUT>(&vals[0], &widths[0], vals.size());
}


template <class TIN, class TOUT>
inline void Unpack(TIN val, const unsigned int * widths, TOUT * unpacked_vals, unsigned int num_fields)
{
  TIN working_value = val;

  unsigned int total_width = 0; // not used in computation, just checking
  for (unsigned int i = 0; i < num_fields; i++) {

    unsigned int field_width = widths[i];
    TIN one = static_cast<TIN>(1);

    TOUT field_val = working_value % (one << field_width);
    working_value = working_value >> field_width;
    assert(working_value < std::numeric_limits<TIN>::max());

    assert(field_val < std::numeric_limits<TOUT>::max());
    assert(field_val < (static_cast<TOUT>(1) << field_width));
    unpacked_vals[i] = field_val;

    total_width += field_width;
    assert(total_width <= 64 && "total sum of widths exceeds 64");
  }
}

template <class TIN, class TOUT>
inline std::vector<TOUT> UnpackV(TIN val, const std::vector<unsigned int> & widths)
{
  std::vector<TOUT> unpacked_vals(widths.size(), 0);
  Unpack<TIN, TOUT>(val, &widths[0], &unpacked_vals[0], widths.size());
  return unpacked_vals;
}


template <class TIN, class TOUT>
inline std::pair<unsigned int, TOUT> DecodeFH(
    TIN input, 
    const std::vector<TIN> & leaf_routes, 
    const std::vector<TIN> & leaf_route_masks, 
    const std::vector<TIN> & leaf_payload_masks,
    const std::vector<unsigned int> & payload_shifts
)
{
  assert(leaf_routes.size() == leaf_route_masks.size());
  assert(leaf_routes.size() == leaf_payload_masks.size());
  assert(leaf_routes.size() == payload_shifts.size());

  bool had_some_match; // used to make sure route table is well-formed
  for (unsigned int i = 0; i < leaf_routes.size(); i++) {

    // iterate through all the input vectors together, over leaves
    TIN route = leaf_routes[i];
    TIN route_mask = leaf_route_masks[i];
    TIN payload_mask = leaf_payload_masks[i];
    unsigned int payload_shift = payload_shifts[i];

    // mask out what the route and payload would be if this were the correct leaf
    TIN potential_route_bits = input & route_mask;
    TIN potential_payload_bits = input & payload_mask;

    // compare route bits to actual route
    bool match = (potential_route_bits == route);

    assert(!(had_some_match && match) && "input matched more than one route");
    had_some_match |= match;

    if (match) {
      TOUT payload = static_cast<TOUT>(potential_payload_bits >> payload_shift);
      unsigned int leaf_id = i;

      return {leaf_id, payload};
    }
  }

  assert(false && "input matched no routes");
  return {0, 0};
}


template <class TIN, class TOUT>
std::pair<std::vector<TOUT>, unsigned int> SerializeWord2(TIN input, unsigned int input_width)
{
  unsigned int rem = input_width % 2;
  unsigned int half_width = input_width >> 1;
  unsigned int half_chunk_width = half_width + rem;

  std::vector<TOUT> unpacked;
  if (rem == 0) {
    unpacked = UnpackV<TIN, TOUT>(input, {half_width, half_width});
  } else {
    unpacked = UnpackV<TIN, TOUT>(input, {half_width + 1, half_width});
  }

  return {
    {unpacked[0], unpacked[1]}, 
    half_chunk_width
  };
}

template <class TIN, class TOUT>
std::pair<std::vector<TOUT>, unsigned int> SerializeWord4(TIN input, unsigned int input_width)
{
  // if you had to recurse, base the i+1 off this
  // but BD serializers are never deeper than 4 so...
  
  std::vector<TOUT> chunks;
  unsigned int chunk_width;
  std::tie(chunks, chunk_width) = SerializeWord2<TIN, TOUT>(input, input_width);

  std::vector<TOUT> retval01;
  unsigned int chunk_width01;
  std::tie(retval01, chunk_width01) = SerializeWord2<TOUT, TOUT>(chunks[0], chunk_width);

  std::vector<TOUT> retval23;
  unsigned int chunk_width23;
  std::tie(retval23, chunk_width23) = SerializeWord2<TOUT, TOUT>(chunks[1], chunk_width);
  
  assert(chunk_width01 == chunk_width23);

  return {
    {retval01[0], retval01[1], retval23[0], retval23[1]}, 
    chunk_width01
  };

}

template <class TIN, class TOUT>
std::pair<std::vector<TOUT>, unsigned int> SerializeWords(const std::vector<TIN> & inputs, unsigned int input_width, unsigned int serialization)
{
  // return values
  std::vector<TOUT> serialized_payloads;
  serialized_payloads.reserve(inputs.size() * serialization);
  unsigned int serialized_width;

  if (serialization == 1) {

    // XXX this isn't ideal, vector creation adds overhead
    for (auto& input : inputs) {
      serialized_payloads.push_back(static_cast<TOUT>(input));
    }
    serialized_width = input_width;

  } else if (serialization == 2) {

    std::vector<TOUT> chunks;
    unsigned int chunk_width;
    for (auto& input : inputs) {
      std::tie(chunks, chunk_width) = SerializeWord2<TIN, TOUT>(input, input_width);
      for (auto& chunk : chunks) {
        serialized_payloads.push_back(chunk);
      }
    }
    serialized_width = chunk_width;

  } else if (serialization == 4) {

    std::vector<TOUT> chunks;
    unsigned int chunk_width;
    for (auto& input : inputs) {
      std::tie(chunks, chunk_width) = SerializeWord4<TIN, TOUT>(input, input_width);
      for (auto& chunk : chunks) {
        serialized_payloads.push_back(chunk);
      }
    }
    serialized_width = chunk_width;

  } else {
    assert(false && "serialization must be one of {1, 2, 4}");
  }

  return {serialized_payloads, serialized_width};
}


template <class TIN, class TOUT>
TOUT DeserializeWord2(std::vector<TIN> inputs, unsigned int output_width)
{
  assert(inputs.size() == 2);

  unsigned int half_width = output_width >> 1;
  unsigned int remainder = output_width % 2;

  uint64_t packed_word = PackV<TIN, TOUT>(
      {inputs[0], inputs[1]}, 
      {half_width + remainder, half_width}
  );

  return packed_word;
}

template <class TIN, class TOUT>
TOUT DeserializeWord4(std::vector<TIN> inputs, unsigned int output_width)
{
  assert(inputs.size() == 4);

  std::vector<TIN> intermediate_chunks;

  unsigned int half_width = output_width >> 1;
  unsigned int remainder = output_width % 2;

  intermediate_chunks.push_back(DeserializeWord2<TIN, TIN>({inputs[0], inputs[1]}, half_width + remainder));
  intermediate_chunks.push_back(DeserializeWord2<TIN, TIN>({inputs[2], inputs[3]}, half_width));

  return DeserializeWord2<TIN, TOUT>(intermediate_chunks, output_width);
}

template <class TIN, class TOUT>
std::pair<std::vector<TOUT>, std::vector<TIN> > DeserializeWords(const std::vector<TIN> & inputs, unsigned int output_width, unsigned int deserialization)
{

  std::vector<TOUT> deserialized_words;
  std::vector<TIN> remainder;
  
  if (deserialization == 1) {

    // XXX this isn't ideal, adds overhead
    for (auto& input : inputs) {
      deserialized_words.push_back(static_cast<TOUT>(input));
    }
    
  } else if (deserialization == 2) {

    for (unsigned int i = 0; i < inputs.size() / 2; i++) {
      deserialized_words.push_back(DeserializeWord2<TIN, TOUT>({inputs[2*i], inputs[2*i + 1]}, output_width));
    }

  } else if (deserialization == 4) {

    for (unsigned int i = 0; i < inputs.size() / 4; i++) {
      deserialized_words.push_back(
          DeserializeWord4<TIN, TOUT>(
            {inputs[4*i + 0], inputs[4*i + 1], inputs[4*i + 2], inputs[4*i + 3]}, 
            output_width
          )
      );
    }

  } else {
    assert(false && "deserialization must be one of {1, 2}");
  }

  // there might be a remainder
  for (unsigned int i = inputs.size() % deserialization; i >= 1; i--) { // note weird iterator
    remainder.push_back(inputs.at(inputs.size() - i));
  }

  return {deserialized_words, remainder};
}


template <class T>
std::string UintAsString(T value, unsigned int width)
{
  std::string str;
  T working_val = value;

  for (unsigned int i = 0; i < width; i++) {
    char bitchar; 
    if (working_val % 2 == 0) {
      bitchar = '0';
    } else {
      bitchar = '1';
    }

    str.insert(str.begin(), bitchar); // prepend (there's no push_front)

    working_val = working_val >> 1;
  }

  return str;
}

} // bddriver
} // pystorm
