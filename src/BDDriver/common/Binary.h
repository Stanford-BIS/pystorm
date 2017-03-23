#ifndef BINARY_H
#define BINARY_H

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

#include "BDDriver/common/util.h"

namespace pystorm {

class Binary {
  public:
    // single field
    Binary(uint64_t value, uint8_t width);
    // construct values_ from uints
    Binary(const std::vector<uint64_t>& values,
           const std::vector<uint8_t>& widths);
    // concatenate other Binarys to make Binary
    Binary(const std::vector<Binary>& binarys);

    uint64_t TotalWidth() const; 

    // pack values into a single uint64_t
    inline uint64_t AsUint() const { return util::PackUint(values_, widths_); }
    // return values as a string    
    inline std::string AsString() const { return util::UintAsString(AsUint(), TotalWidth()); }
    // unpack into different-width fields
    inline std::vector<uint64_t> Unpack(std::vector<uint8_t> field_widths) const { return util::UnpackUint(AsUint(), field_widths); }

  private:
    std::vector<uint64_t> values_;
    std::vector<uint8_t> widths_;

}; // Binary

} // pystorm

#endif
