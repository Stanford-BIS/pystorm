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

Binary::Binary(const std::vector<std::string>& field_names,
               const std::vector<uint64_t>& values,
               const std::vector<uint8_t>& widths)
{
  assert(field_names.size() == values.size() && field_names.size() == widths.size() && "vector lengths must match");

  values_ = values;
  widths_ = widths;

  total_width_ = 0;

  for (int i = 0; i < field_names.size(); i++) {
    value_map_[field_names[i]] = values[i];
    width_map_[field_names[i]] = widths[i];
    total_width_ += widths[i];
  }
}

Binary::Binary(const std::vector<Binary>& binarys)
{
  std::vector<uint64_t> all_values;
  std::vector<uint8_t> all_widths;
  std::vector<std::string> all_names;
  for (auto& el : binarys) {
    for (int idx = 0; idx < el.values_.size(); idx++) {
      all_values.push_back(el.values_[idx]);
      all_widths.push_back(el.widths_[idx]);
      all_names.push_back(el.field_names_[idx]);
    }
  }

  Binary(all_names, all_values, all_widths);
}

} // pystorm
