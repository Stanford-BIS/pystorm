#ifndef DRIVER_UTIL_H
#define DRIVER_UTIL_H

#include <cstdint>
#include <vector>
#include <string>

namespace pystorm {
namespace util {

uint64_t PackUint(const std::vector<uint64_t>& vals, const std::vector<uint8_t>& widths);

std::vector<uint64_t> UnpackUint(uint64_t value, const std::vector<uint8_t>& widths);

std::string UintAsString(uint64_t value, uint8_t width);

} // util
} // pystorm
#endif
