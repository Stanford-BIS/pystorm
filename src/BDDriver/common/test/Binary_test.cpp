#include <cstdint>
#include <iostream>
#include <vector>

#include "BDDriver/common/util.h"
#include "BDDriver/common/Binary.h"

using std::cout;
using std::endl;
using namespace pystorm;

int main () {
    
  std::vector<uint64_t> vals = {1, 1};
  std::vector<uint8_t> widths = {2, 2};

  uint64_t my_uint = util::PackUint(vals, widths);

  std::vector<uint64_t> unpacked_vals = util::UnpackUint(my_uint, widths);

  cout << "the integer value " << my_uint << endl;

  cout << "in binary " << util::UintAsString(my_uint, 4) << endl;

  cout << "was made from: ";
  for (auto& it : unpacked_vals) {
    cout << it << ", ";
  }
  cout << endl;

  cout << "using Binary class:" << endl;

  auto my_bin = Binary(vals, widths);
  cout << "total width " << my_bin.TotalWidth() << endl;
  cout << "as int " << my_bin.AsUint() << endl;
  cout << "as string " << my_bin.AsString() << endl;

}
