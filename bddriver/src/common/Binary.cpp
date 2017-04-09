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
  value_ = value;
  width_ = width;
}

Binary::Binary(const std::vector<uint64_t>& values,
               const std::vector<uint8_t>& widths)
{
  assert(values.size() == widths.size() 
    && "vector lengths must match");

  value_ = PackUint(values, widths);

  width_ = 0;
  for (auto& el : widths) {
    width_ += el;
  }

}

Binary::Binary(const std::vector<Binary>& binarys)
{
  std::vector<uint64_t> values;
  std::vector<uint8_t> widths;
  for (auto& el : binarys) {
    values.push_back(el.AsUint());
    widths.push_back(el.Bitwidth());
  }

  value_ = PackUint(values, widths);
  width_ = 0;
  for (auto& el : widths) {
    width_ += el;
  }

}


} // bddriver
} // pystorm
