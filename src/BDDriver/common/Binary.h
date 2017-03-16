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
    // construct values_ from uints
    Binary(const std::vector<std::string>& field_names,
         const std::vector<uint64_t>& values,
         const std::vector<uint8_t>& widths);
    // concatenate other Binarys to make Binary
    Binary(const std::vector<Binary>& binarys);

    inline uint64_t FieldValue(const std::string& str) { return value_map_[str]; }
    inline uint8_t FieldWidth(const std::string& str) { return width_map_[str]; }
    inline unsigned int TotalWidth() { return total_width_; }

    // pack values into a single uint64_t
    inline uint64_t AsUint() { return util::PackUint(values_, widths_); }
    // return values as a string    
    inline std::string AsString() { return util::UintAsString(AsUint(), total_width_); }

  private:
    std::vector<std::string> field_names_;
    std::vector<uint64_t> values_;
    std::vector<uint8_t> widths_;
    std::unordered_map<std::string, uint64_t> value_map_;
    std::unordered_map<std::string, uint8_t> width_map_;
    unsigned int total_width_;

}; // Binary

} // pystorm

#endif
