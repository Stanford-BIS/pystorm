#include "BDDriver/common/Binary.h"

#include <assert.h>
#include <iostream>

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

#include "BDDriver/common/util.h"

namespace pystorm {

using std::cout;
using std::endl;

Binary::Binary(uint64_t value, uint8_t width)
{
  std::vector<uint64_t> value_v = {value};
  std::vector<uint8_t> width_v = {width};
  Binary(value_v, width_v);
}

Binary::Binary(const std::vector<uint64_t>& values,
               const std::vector<uint8_t>& widths)
{
  assert(values.size() == widths.size() 
    && "vector lengths must match");

  values_ = values;
  widths_ = widths;

}

Binary::Binary(const std::vector<Binary>& binarys)
{
  std::vector<uint64_t> all_values;
  std::vector<uint8_t> all_widths;
  for (auto& el : binarys) {
    for (unsigned int idx = 0; idx < el.values_.size(); idx++) {
      all_values.push_back(el.values_[idx]);
      all_widths.push_back(el.widths_[idx]);
    }
  }

  Binary(all_values, all_widths);
}

uint64_t Binary::TotalWidth() const
{
  uint64_t total_width = 0;
  for (auto& width : widths_) {
    total_width += width;
  }
  return total_width;
}


} // pystorm
