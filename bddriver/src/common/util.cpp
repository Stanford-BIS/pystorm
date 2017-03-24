#include "common/util.h"
#include "common/Binary.h"

#include <assert.h>
#include <iostream>

#include <cstdint>
#include <vector>
#include <string>

namespace pystorm {
namespace util {

using std::cout;
using std::endl;

uint64_t PackUint(const std::vector<uint64_t>& vals, const std::vector<uint8_t>& widths)
{
  assert(vals.size() == widths.size() && "vals and widths must have same number of elements");

  uint64_t value = 0;
  uint8_t width = 0;

  for (unsigned int idx = 0; idx < vals.size(); idx++) {

    // important that all operands used are 64b
   
    uint64_t field_val = vals[idx];
    uint64_t field_width = widths[idx];

    uint64_t one = 1;

    assert(field_val < (one << field_width) && "packed element value exceeds value allowed by packed element width");

    value = value | field_val << width;

    width += field_width;
    assert(width <= 64 && "total width of packed elements exceeds 64");
  }
  return value;
}


std::vector<uint64_t> UnpackUint(uint64_t value, const std::vector<uint8_t>& widths)
{
  std::vector<uint64_t> field_vals;
  uint64_t working_value = value;

  uint8_t total_width = 0;
  for (auto& field_width : widths) {

    total_width += field_width;
    assert(total_width < 64 && "total sum of widths exceeds 64");

    uint64_t val_mask = (1 << field_width) - 1; // mask[0:field_width-1] = 1, mask[field_width:] = 0 : e.g. 00001111
    uint64_t field_val = working_value & val_mask;
    working_value = working_value >> field_width;

    field_vals.push_back(field_val);
  }

  return field_vals;
}

std::string UintAsString(uint64_t value, uint8_t width)
{
  std::string str;
  uint64_t working_val = value;

  for (int i = 0; i < width; i++) {
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

} // util
} // pystorm
