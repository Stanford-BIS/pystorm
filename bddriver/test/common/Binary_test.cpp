#include "Binary.h"
#include "util.h"
#include "gtest/gtest.h"

#include <vector>
#include <cstdint>
#include <string>
#include <iostream>

using namespace pystorm;
using namespace bddriver;
using namespace std;

class BinaryFixture : public testing::Test
{
  public:
    void SetUp() {
      vals = {3, 1, 0};
      widths = {2, 2, 2};
      packed_val = 7;
      total_width = 6;

      my_uint = PackUint(vals, widths);
      my_bin = Binary(vals, widths);
    }

    std::vector<uint64_t> vals;
    std::vector<uint8_t> widths;
    uint64_t packed_val;
    uint64_t total_width;
    Binary my_bin;
    uint64_t my_uint;
};

TEST_F(BinaryFixture, TestPack)
{
  EXPECT_EQ(packed_val, my_uint);
}

TEST_F(BinaryFixture, TestUnpack)
{
  std::vector<uint64_t> unpacked_vals = UnpackUint(my_uint, widths);
  EXPECT_EQ(unpacked_vals.size(), widths.size());
  EXPECT_EQ(unpacked_vals.size(), vals.size());
  for (unsigned int i = 0; i < unpacked_vals.size(); i++) {
    EXPECT_EQ(unpacked_vals[i], vals[i]);
  }
}

TEST_F(BinaryFixture, TestAsString)
{
  string bin_str = UintAsString(my_uint, total_width);

  vector<uint64_t> bin_vect;
  vector<uint8_t> ones;
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
  uint64_t bin_vect_as_uint = PackUint(bin_vect, ones);

  EXPECT_EQ(bin_vect_as_uint, my_uint);
}

TEST_F(BinaryFixture, TestBinaryConstructor)
{
  EXPECT_EQ(my_bin.Bitwidth(), total_width);
  EXPECT_EQ(my_bin.AsUint(), packed_val);
  EXPECT_EQ(my_bin.AsUint(), my_uint);
}

