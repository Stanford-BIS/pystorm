#ifndef BINARY_H
#define BINARY_H

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

#include "util.h"

namespace pystorm {
namespace bddriver {

class Binary {
  public:
    Binary() { }
    // single field
    Binary(uint64_t value, uint8_t width);
    // construct values_ from uints
    Binary(const std::vector<uint64_t>& values,
           const std::vector<uint8_t>& widths);
    // concatenate other Binarys to make Binary
    Binary(const std::vector<Binary>& binarys);

    inline uint8_t Bitwidth() const { return width_; }

    // pack values into a single uint64_t
    inline uint64_t AsUint() const { return value_; }
    // return values as a string    
    inline std::string AsString() const { return UintAsString(AsUint(), Bitwidth()); }
    // unpack into different-width fields
    inline std::vector<uint64_t> Unpack(std::vector<uint8_t> field_widths) const { return UnpackUint(AsUint(), field_widths); }

  private:
    uint64_t value_;
    uint8_t width_;

}; // Binary

} // bddriver
} // pystorm

#endif
