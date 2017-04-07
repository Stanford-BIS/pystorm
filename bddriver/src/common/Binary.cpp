#include "Binary.h"

#include <assert.h>
#include <iostream>

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

#include "util.h"

namespace pystorm {
namespace bddriver {

using std::cout;
using std::endl;

Binary::Binary(uint64_t value, uint8_t width)
{
  values_ = {value};
  widths_ = {width};
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
  values_.clear();
  widths_.clear();
  for (auto& el : binarys) {
    for (unsigned int idx = 0; idx < el.values_.size(); idx++) {
      values_.push_back(el.values_[idx]);
      widths_.push_back(el.widths_[idx]);
    }
  }

}

uint64_t Binary::Bitwidth() const
{
  uint64_t total_width = 0;
  for (auto& width : widths_) {
    total_width += width;
  }
  return total_width;
}


} // bddriver
} // pystorm
