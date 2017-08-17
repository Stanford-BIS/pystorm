#include "BDPars.h"

#include <iostream>
using std::cout;
using std::endl;

#include "gtest/gtest.h"

using namespace pystorm;
using namespace bddriver;

constexpr uint64_t     InputTag::field_hard_values[];
constexpr unsigned int InputTag::field_widths[];

class BDParsFixture : public testing::Test {
 public:
  void SetUp() { pars = new bdpars::BDPars(); }
  void TearDown() { delete pars; }

  bdpars::BDPars* pars;
};

TEST_F(BDParsFixture, TestSetUpTearDown) {}

TEST(BDWord, CreateInputTag) {
  BDWord my_word = BDWord::Create<InputTag>({{InputTag::COUNT, 1}, {InputTag::TAG, 3}});
  cout << InputTag::field_widths[0] << ", " << InputTag::field_widths[1] << endl;
  uint32_t packed_val = my_word.Packed();
  cout << packed_val << endl;
  cout << UintAsString(packed_val, 20) << endl;

  unsigned int count = my_word.At<InputTag, unsigned int>(InputTag::COUNT);
  unsigned int tag = my_word.At<InputTag>(InputTag::TAG);

  cout << "COUNT " << count << endl;
  cout << "TAG " << tag << endl;
}




