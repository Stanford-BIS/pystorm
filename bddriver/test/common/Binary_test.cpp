#include "binary_util.h"
#include "gtest/gtest.h"

#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <utility>
#include <vector>
using std::cout;
using std::endl;

using namespace pystorm;
using namespace bddriver;

class BinaryFixtureRandom : public testing::TestWithParam<unsigned int>

{
 public:
  void SetUp() {
    auto generator                = std::default_random_engine(0);
    auto uint_distribution        = std::uniform_int_distribution<uint64_t>(0, UINT64_MAX);
    auto field_width_distribution = std::uniform_int_distribution<unsigned int>(1, 64);

    kMaxFieldWidth = GetParam();

    for (unsigned int i = 0; i < M; i++) {
      initial_field_width.push_back(field_width_distribution(generator));
      uint64_t new_val = uint_distribution(generator) % (static_cast<uint64_t>(1) << initial_field_width.back());
      vals.push_back(new_val);

      sub_field_widths.push_back({});

      unsigned int total_field_width = 0;
      while (total_field_width < initial_field_width.back()) {
        unsigned int sub_field_width = field_width_distribution(generator) % kMaxFieldWidth;
        unsigned int next_field_width;
        if (total_field_width + sub_field_width < initial_field_width.back()) {
          next_field_width = sub_field_width;
        } else {
          next_field_width = initial_field_width.back() - total_field_width;
        }
        sub_field_widths.back().push_back(next_field_width);
        total_field_width += next_field_width;
      }
      assert(total_field_width == initial_field_width.back());
    }
  }

  const unsigned int M = 100;
  unsigned int kMaxFieldWidth;
  std::vector<uint64_t> vals;
  std::vector<unsigned int> initial_field_width;
  std::vector<std::vector<unsigned int> > sub_field_widths;
};

TEST_P(BinaryFixtureRandom, TestUnpackPack) {
  for (unsigned int i = 0; i < initial_field_width.size(); i++) {
    uint64_t new_val;
    if (kMaxFieldWidth > 32) {
      std::vector<uint64_t> unpacked_vals = UnpackV<uint64_t, uint64_t>(vals[i], sub_field_widths[i]);
      new_val                             = PackV<uint64_t, uint64_t>(unpacked_vals, sub_field_widths[i]);
    } else {
      // cout << "i = " << i << endl;
      // cout << "val = " << vals[i] << ", width = " << initial_field_width[i] << endl;
      std::vector<uint32_t> unpacked_vals = UnpackV<uint64_t, uint32_t>(vals[i], sub_field_widths[i]);
      // cout << "unpacked = {";
      // unsigned int j = 0;
      // for (auto& it : unpacked_vals) {
      //  cout << it << "(" << sub_field_widths[i][j++] << ") ";
      //}
      // cout << "}" << endl;
      new_val = PackV<uint32_t, uint64_t>(unpacked_vals, sub_field_widths[i]);
      // cout << "new val = " << new_val << endl;
    }
    ASSERT_EQ(vals[i], new_val);
  }
}

INSTANTIATE_TEST_CASE_P(TestBinaryDifferentUintN_t, BinaryFixtureRandom, ::testing::Values(10, 20, 40));

class BinaryFixture : public testing::Test {
 public:
  void SetUp() {
    vals        = {3, 1, 0};
    widths      = {2, 2, 2};
    packed_val  = 7;
    total_width = 6;

    my_uint = PackV64(vals, widths);
  }

  std::vector<uint64_t> vals;
  std::vector<unsigned int> widths;
  uint64_t packed_val;
  uint64_t total_width;
  uint64_t my_uint;
};

TEST_F(BinaryFixture, TestPack) { EXPECT_EQ(packed_val, my_uint); }

TEST_F(BinaryFixture, TestUnpack) {
  std::vector<uint64_t> unpacked_vals = UnpackV64(my_uint, widths);
  EXPECT_EQ(unpacked_vals.size(), widths.size());
  EXPECT_EQ(unpacked_vals.size(), vals.size());
  for (unsigned int i = 0; i < unpacked_vals.size(); i++) {
    EXPECT_EQ(unpacked_vals[i], vals[i]);
  }
}

TEST_F(BinaryFixture, TestAsString) {
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

class BinaryDesSerFixture : public testing::TestWithParam<std::pair<unsigned int, unsigned int> > {
 public:
  void SetUp() {
    std::tie(serialization, output_width) = GetParam();

    std::default_random_engine generator(0);
    std::uniform_int_distribution<uint32_t> distribution(0, UINT32_MAX);

    // figure out how big the inputs should be for this output_width
    std::vector<unsigned int> input_width_sequence;

    unsigned int half_width2_0 = output_width / 2 + output_width % 2;
    unsigned int half_width2_1 = output_width / 2;

    unsigned int half_width4_0 = half_width2_0 / 2 + half_width2_0 % 2;
    unsigned int half_width4_1 = half_width2_0 / 2;
    unsigned int half_width4_2 = half_width2_1 / 2 + half_width2_1 % 2;
    unsigned int half_width4_3 = half_width2_1 / 2;

    if (serialization == 1) {
      input_width_sequence = {output_width};
    } else if (serialization == 2) {
      input_width_sequence = {half_width2_0, half_width2_1};
    } else if (serialization == 4) {
      input_width_sequence = {half_width4_0, half_width4_1, half_width4_2, half_width4_3};
    } else {
      assert(false);
    }

    // generate random inputs with widths from input_width_sequence
    for (unsigned int i = 0; i < N; i++) {
      unsigned int input_width = input_width_sequence[i % serialization];
      uint32_t input           = distribution(generator) % input_width;
      inputs.push_back(input);
    }
  }

  const unsigned int N = 103;

  unsigned int serialization;
  unsigned int output_width;

  std::vector<uint32_t> inputs;
};

TEST_P(BinaryDesSerFixture, TestDesSer) {
  std::vector<uint64_t> deserialized;
  std::vector<uint32_t> remainder;
  std::tie(deserialized, remainder) = DeserializeWords<uint32_t, uint64_t>(inputs, output_width, serialization);

  std::vector<uint32_t> serialized;
  unsigned int serialized_width;
  std::tie(serialized, serialized_width) =
      SerializeWords<uint64_t, uint32_t>(deserialized, output_width, serialization);

  // XXX should test intermediate results

  // we might have a remainder

  ASSERT_EQ(inputs.size(), serialized.size() + remainder.size());
  for (unsigned int i = 0; i < serialized.size(); i++) {
    ASSERT_EQ(inputs[i], serialized[i]);
  }
  for (unsigned int i = 0; i < remainder.size(); i++) {
    ASSERT_EQ(inputs[serialized.size() + i], remainder[i]);
  }
}

INSTANTIATE_TEST_CASE_P(DifferentSerializationBitwidth,
                        BinaryDesSerFixture,
                        ::testing::Values(
                            std::make_pair(1, 8),
                            std::make_pair(2, 8),
                            std::make_pair(2, 9),
                            std::make_pair(4, 8),
                            std::make_pair(4, 10),
                            std::make_pair(4, 11)););

class BinarySerDesFixture : public testing::TestWithParam<std::pair<unsigned int, unsigned int> > {
 public:
  void SetUp() {
    std::tie(serialization, input_width) = GetParam();

    std::default_random_engine generator(0);
    std::uniform_int_distribution<uint64_t> distribution(0, UINT64_MAX);

    for (unsigned int i = 0; i < N; i++) {
      uint64_t input = distribution(generator) % input_width;
      inputs.push_back(input);
    }
  }

  const unsigned int N = 100;

  unsigned int serialization;
  unsigned int input_width;

  std::vector<uint64_t> inputs;
};

TEST_P(BinarySerDesFixture, TestSerDes) {
  std::vector<uint32_t> serialized;
  unsigned int serialized_width;
  std::tie(serialized, serialized_width) = SerializeWords<uint64_t, uint32_t>(inputs, input_width, serialization);
  std::vector<uint64_t> deserialized;
  std::vector<uint32_t> remainder;
  std::tie(deserialized, remainder) = DeserializeWords<uint32_t, uint64_t>(serialized, input_width, serialization);

  // XXX should test intermediate results

  ASSERT_EQ(inputs.size(), deserialized.size());
  ASSERT_EQ(remainder.size(), static_cast<unsigned int>(0));
  for (unsigned int i = 0; i < inputs.size(); i++) {
    ASSERT_EQ(inputs[i], deserialized[i]);
  }
}

INSTANTIATE_TEST_CASE_P(DifferentSerializationBitwidth,
                        BinarySerDesFixture,
                        ::testing::Values(
                            std::make_pair(1, 8),
                            std::make_pair(2, 8),
                            std::make_pair(2, 9),
                            std::make_pair(4, 8),
                            std::make_pair(4, 10),
                            std::make_pair(4, 11)););
