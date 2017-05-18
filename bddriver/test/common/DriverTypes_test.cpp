#include "DriverTypes.h"

#include <cstdint>
#include <iostream>
using std::cout;
using std::endl;

#include "test_util/DriverTypes_util.h"
#include "gtest/gtest.h"

using namespace pystorm;
using namespace bddriver;

class DriverTypesFixture : public testing::Test
{
  public:
    void SetUp() {
    }
    void TearDown() {
    }

    const unsigned int N = 1000;

};

// for each of these tests, just convert back and forth and make sure that what you get 
// is the same as what you put in.
// XXX it would be better to verify the FieldVValues, too

TEST_F(DriverTypesFixture, PATData) {
  std::vector<PATData> data = MakeRandomPATData(N);

  auto fvv = DataToFieldVValues(data);
  std::vector<PATData> data_out = FieldVValuesToPATData(fvv);

  ComparePATData(data, data_out);
}

TEST_F(DriverTypesFixture, TATData) {
  std::vector<TATData> data = MakeRandomTATData(N);

  auto fvv = DataToFieldVValues(data);
  std::vector<TATData> data_out = FieldVValuesToTATData(fvv);

  CompareTATData(data, data_out);
}

TEST_F(DriverTypesFixture, AMData) {
  std::vector<AMData> data = MakeRandomAMData(N);

  auto fvv = DataToFieldVValues(data);
  std::vector<AMData> data_out = FieldVValuesToAMData(fvv);

  CompareAMData(data, data_out);
}

TEST_F(DriverTypesFixture, MMData) {
  std::vector<MMData> data = MakeRandomMMData(N);

  auto fvv = DataToFieldVValues(data);
  std::vector<MMData> data_out = FieldVValuesToMMData(fvv);

  CompareMMData(data, data_out);
}