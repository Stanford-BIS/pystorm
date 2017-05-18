#ifndef DRIVER_UTIL_H
#define DRIVER_UTIL_H

#include <cstdint>
#include <vector>
#include <string>

namespace pystorm {
namespace bddriver {

// bit-packing functions 
template <class TIN, class TOUT>
TOUT PackV(const std::vector<TIN> & vals, const std::vector<unsigned int> & widths);

template <class TIN, class TOUT>
TOUT Pack(const TIN * vals, const unsigned int * widths, unsigned int num_fields);

// bit-unpacking functions
template <class TIN, class TOUT>
std::vector<TOUT> UnpackV(TIN val, const std::vector<unsigned int> & widths);

template <class TIN, class TOUT>
void Unpack(TIN val, const unsigned int * widths, TOUT * unpacked_vals, unsigned int num_fields);

// funnel/horn decoding (encoding is just packing)

template <class TIN, class TOUT>
inline std::pair<unsigned int, TOUT> DecodeFH(
    TIN input, 
    const std::vector<TIN> & leaf_routes, 
    const std::vector<TIN> & leaf_route_masks, 
    const std::vector<TIN> & leaf_payload_masks,
    const std::vector<unsigned int> & payload_shifts
);

// misc helpers
template <class T>
std::string AsString(T value, unsigned int width);

inline uint64_t MaxVal(unsigned int width) { uint64_t one = 1; return (one << width) - 1; }

// XXX The *only* thing the following does is let me write "Pack64" instead of "Pack<uint64_t>"
// I suppose it also hints at which types the template functions are supposed to be used with

// these are function pointers, first lines declare function signature for each pointer
// second lines assign to templated call

///////////////
// 64-bit

// PackV64
typedef uint64_t (*PackV64_type)(const std::vector<uint64_t> &, const std::vector<unsigned int> &);
PackV64_type const PackV64 = &PackV<uint64_t, uint64_t>;

// Pack64
typedef uint64_t (*Pack64_type)(const uint64_t * vals, const unsigned int * widths, unsigned int num_fields);
Pack64_type const Pack64 = &Pack<uint64_t, uint64_t>;

// UnpackV64
typedef std::vector<uint64_t> (*UnpackV64_type)(uint64_t val, const std::vector<unsigned int> & widths);
UnpackV64_type const UnpackV64 = &UnpackV<uint64_t, uint64_t>;

// Unpack64
typedef void (*Unpack64_type)(uint64_t val, const unsigned int * widths, uint64_t * unpacked_vals, unsigned int num_fields);
Unpack64_type const Unpack64 = &Unpack<uint64_t, uint64_t>;

// UnpackV64to32
typedef std::vector<uint32_t> (*UnpackV64to32_type)(uint64_t val, const std::vector<unsigned int> & widths);
UnpackV64to32_type const UnpackV64to32 = &UnpackV<uint64_t, uint32_t>;

// Unpack64to32
typedef void (*Unpack64to32_type)(uint64_t val, const unsigned int * widths, uint32_t * unpacked_vals, unsigned int num_fields);
Unpack64to32_type const Unpack64to32 = &Unpack<uint64_t, uint32_t>;

///////////////
// 32-bit

// PackV32
typedef uint32_t (*PackV32_type)(const std::vector<uint32_t> &, const std::vector<unsigned int> &);
PackV32_type const PackV32 = &PackV<uint32_t, uint32_t>;

// Pack32
typedef uint32_t (*Pack32_type)(const uint32_t * vals, const unsigned int * widths, unsigned int num_fields);
Pack32_type const Pack32 = &Pack<uint32_t, uint32_t>;

// UnpackV32
typedef std::vector<uint32_t> (*UnpackV32_type)(uint32_t val, const std::vector<unsigned int> & widths);
UnpackV32_type const UnpackV32 = &UnpackV<uint32_t, uint32_t>;

// Unpack32
typedef void (*Unpack32_type)(uint32_t val, const unsigned int * widths, uint32_t * unpacked_vals, unsigned int num_fields);
Unpack32_type const Unpack32 = &Unpack<uint32_t, uint32_t>;

// PackV32to64
typedef uint64_t (*PackV32to64_type)(const std::vector<uint32_t> &, const std::vector<unsigned int> &);
PackV32to64_type const PackV32to64 = &PackV<uint32_t, uint64_t>;

// Pack32to64
typedef uint64_t (*Pack32to64_type)(const uint32_t * vals, const unsigned int * widths, unsigned int num_fields);
Pack32to64_type const Pack32to64 = &Pack<uint32_t, uint64_t>;


} // bddriver
} // pystorm

// templated functions, so include source
#include "binary_util.cpp"

#endif
