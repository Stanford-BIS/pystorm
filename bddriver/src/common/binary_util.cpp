#include <assert.h>
#include <iostream>

#include <cstdint>
#include <vector>
#include <string>

namespace pystorm {
namespace bddriver {

using std::cout;
using std::endl;

template <class T>
T Pack(const T * vals, const unsigned int * widths, unsigned int num_fields)
{
  T value = 0;
  unsigned int width = 0;

  for (unsigned int idx = 0; idx < num_fields; idx++) {

    T field_val = vals[idx];
    T field_width = widths[idx];

    T one = 1;

    assert(field_val < (one << field_width) && "packed element value exceeds value allowed by packed element width");

    value = value | field_val << width;

    width += field_width;
    assert(width <= 64 && "total width of packed elements exceeds 64");
  }
  return value;
}

template <class T> 
T PackV(const std::vector<T> & vals, const std::vector<unsigned int> & widths)
{
  assert(vals.size() == widths.size() && "size of values and widths does not match");
  return Pack(&vals[0], &widths[0], vals.size());
}


template <class T>
void Unpack(T val, const unsigned int * widths, T * unpacked_vals, unsigned int num_fields)
{
  T working_value = val;

  unsigned int total_width = 0;
  for (unsigned int i = 0; i < num_fields; i++) {

    unsigned int field_width = widths[i];
    total_width += field_width;
    assert(total_width < 64 && "total sum of widths exceeds 64");

    T val_mask = (1 << field_width) - 1; // mask[0:field_width-1] = 1, mask[field_width:] = 0 : e.g. 00001111
    T field_val = working_value & val_mask;
    working_value = working_value >> field_width;

    unpacked_vals[i] = field_val;
  }
}

template <class T>
std::vector<T> UnpackV(T val, const std::vector<unsigned int> & widths)
{
  std::vector<T> unpacked_vals(widths.size(), 0);
  Unpack(val, &widths[0], &unpacked_vals[0], widths.size());
  return unpacked_vals;
}

template <class T>
std::string UintAsString(T value, unsigned int width)
{
  std::string str;
  T working_val = value;

  for (unsigned int i = 0; i < width; i++) {
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

} // bddriver
} // pystorm
