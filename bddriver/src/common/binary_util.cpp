#include <assert.h>
#include <iostream>

#include <cstdint>
#include <vector>
#include <string>

namespace pystorm {
namespace bddriver {

using std::cout;
using std::endl;

template <class TIN, class TOUT>
inline TOUT Pack(const TIN * vals, const unsigned int * widths, unsigned int num_fields)
{
  TOUT value = 0;
  unsigned int width = 0;

  for (unsigned int idx = 0; idx < num_fields; idx++) {

    TIN field_val = vals[idx];
    TIN field_width = widths[idx];

    assert(field_val < (static_cast<TIN>(1) << field_width) && "packed element value exceeds value allowed by packed element width");

    value = value | static_cast<TOUT>(field_val << width);

    width += field_width;
    assert(width <= 64 && "total width of packed elements exceeds 64");
  }
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

  unsigned int total_width = 0;
  for (unsigned int i = 0; i < num_fields; i++) {

    unsigned int field_width = widths[i];
    total_width += field_width;
    assert(total_width < 64 && "total sum of widths exceeds 64");

    TIN val_mask = (1 << field_width) - 1; // mask[0:field_width-1] = 1, mask[field_width:] = 0 : e.g. 00001111
    TOUT field_val = static_cast<TOUT>(working_value & val_mask);
    working_value = working_value >> field_width;

    unpacked_vals[i] = field_val;
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
