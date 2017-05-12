#include "binary_util.h"
#include "gtest/gtest.h"

#include <vector>
#include <cstdint>
#include <string>
#include <iostream>
using std::cout;
using std::endl;

using namespace pystorm;
using namespace bddriver;


class BinaryFixture : public testing::Test
{
  public:
    void SetUp() {
      vals = {3, 1, 0};
      widths = {2, 2, 2};
      packed_val = 7;
      total_width = 6;

      my_uint = PackV64(vals, widths);
    }

    std::vector<uint64_t> vals;
    std::vector<unsigned int> widths;
    uint64_t packed_val;
    uint64_t total_width;
    uint64_t my_uint;
};

TEST_F(BinaryFixture, TestPack)
{
  EXPECT_EQ(packed_val, my_uint);
}

TEST_F(BinaryFixture, TestUnpack)
{
  std::vector<uint64_t> unpacked_vals = UnpackV64(my_uint, widths);
  EXPECT_EQ(unpacked_vals.size(), widths.size());
  EXPECT_EQ(unpacked_vals.size(), vals.size());
  for (unsigned int i = 0; i < unpacked_vals.size(); i++) {
    EXPECT_EQ(unpacked_vals[i], vals[i]);
  }
}

TEST_F(BinaryFixture, TestAsString)
{
  std::string bin_str = UintAsString<uint64_t>(my_uint, total_width);

  std::vector<uint64_t> bin_vect;
  std::vector<unsigned int> ones;
  EXPECT_EQ(bin_str.size(), total_width);
  for (auto el = bin_str.rbegin(); el != bin_str.rend(); el++) {
    if (*el == '1') {
      bin_vect.push_back(1);
    } else if (*el == '0') {
      bin_vect.push_back(0);
    } else {
      EXPECT_TRUE(false);
    }
    ones.push_back(1);
  }
  cout << endl;
  uint64_t bin_vect_as_uint = PackV64(bin_vect, ones);

  EXPECT_EQ(bin_vect_as_uint, my_uint);
}

