#include "DriverTypes.h"

#include<limits>
#include<cstdint>
#include<random>
#include <iostream>
using std::cout;
using std::endl;

#include "gtest/gtest.h"

using namespace pystorm;
using namespace bddriver;

class DriverTypesFixture : public testing::Test
{
  public:
    void SetUp() {
      generator = std::default_random_engine(0);
      uint_distribution = std::uniform_int_distribution<unsigned int>(0, std::numeric_limits<unsigned int>::max());
    }
    void TearDown() {
    }

    const unsigned int N = 1000;

    std::default_random_engine generator;
    std::uniform_int_distribution<unsigned int> uint_distribution;

    std::vector<PATData> MakeRandomPATData() 
    {
      std::vector<PATData> data;
      data.reserve(N);
      for (unsigned int i = 0; i < N; i++) {
        data[i] = {
          uint_distribution(generator), 
          uint_distribution(generator), 
          uint_distribution(generator)
        };
      }
      return data;
    }
    
    std::vector<TATData> MakeRandomTATData() 
    {
      std::vector<TATData> data;
      data.reserve(N);
      for (unsigned int i = 0; i < N; i++) {
        unsigned int stop = uint_distribution(generator) % 2;
        unsigned int type = uint_distribution(generator) % 3;
        if (type == 0) {
          data[i] = {
            stop, 
            type,
            uint_distribution(generator), 
            uint_distribution(generator), 
            0,
            0,
            0,
            0,
            0,
            0
          };
        } else if (type == 1) {
          int sign0 = 2*(uint_distribution(generator) % 2) - 1;
          int sign1 = 2*(uint_distribution(generator) % 2) - 1;
          data[i] = {
            stop, 
            type,
            0,
            0,
            uint_distribution(generator), 
            sign0,
            uint_distribution(generator), 
            sign1,
            0,
            0
          };

        } else if (type == 2) {
          data[i] = {
            stop, 
            type,
            0,
            0,
            0,
            0,
            0,
            0,
            uint_distribution(generator), 
            uint_distribution(generator)
          };
        } else {
          assert(false);
        }
      }
      return data;
    }

    std::vector<AMData> MakeRandomAMData() 
    {
      std::vector<AMData> data;
      data.reserve(N);
      for (unsigned int i = 0; i < N; i++) {
        unsigned int stop = uint_distribution(generator) % 2;
        data[i] = {
          uint_distribution(generator), 
          stop,
          uint_distribution(generator)
        };
      }
      return data;
    }

    std::vector<MMData> MakeRandomMMData() 
    {
      std::vector<MMData> data;
      data.reserve(N);
      for (unsigned int i = 0; i < N; i++) {
        data[i] = uint_distribution(generator);
      };
      return data;
    }

    void ComparePATData(const std::vector<PATData> & a, const std::vector<PATData> & b)
    {
      ASSERT_EQ(a.size(), b.size());
      for (unsigned int i = 0; i < a.size(); i++) {
        ASSERT_EQ(a[i].AM_address, b[i].AM_address);
        ASSERT_EQ(a[i].MM_address_lo, b[i].MM_address_lo);
        ASSERT_EQ(a[i].MM_address_hi, b[i].MM_address_hi);
      };
    }

    void CompareTATData(const std::vector<TATData> & a, const std::vector<TATData> & b)
    {
      ASSERT_EQ(a.size(), b.size());
      for (unsigned int i = 0; i < a.size(); i++) {
        ASSERT_EQ(a[i].stop, b[i].stop);
        ASSERT_EQ(a[i].type, b[i].type);
        ASSERT_EQ(a[i].AM_address, b[i].AM_address);
        ASSERT_EQ(a[i].MM_address, b[i].MM_address);
        ASSERT_EQ(a[i].synapse_address_0, b[i].synapse_address_0);
        ASSERT_EQ(a[i].synapse_sign_0, b[i].synapse_sign_0);
        ASSERT_EQ(a[i].synapse_address_1, b[i].synapse_address_1);
        ASSERT_EQ(a[i].synapse_sign_1, b[i].synapse_sign_1);
        ASSERT_EQ(a[i].tag, b[i].tag);
        ASSERT_EQ(a[i].global_route, b[i].global_route);
      };
    }

    void CompareAMData(const std::vector<AMData> & a, const std::vector<AMData> & b)
    {
      ASSERT_EQ(a.size(), b.size());
      for (unsigned int i = 0; i < a.size(); i++) {
        ASSERT_EQ(a[i].threshold, b[i].threshold);
        ASSERT_EQ(a[i].stop, b[i].stop);
        ASSERT_EQ(a[i].next_address, b[i].next_address);
      };
    }

    void CompareMMData(const std::vector<MMData> & a, const std::vector<MMData> & b)
    {
      ASSERT_EQ(a.size(), b.size());
      for (unsigned int i = 0; i < a.size(); i++) {
        ASSERT_EQ(a[i], b[i]);
      };
    }
};

// for each of these tests, just convert back and forth and make sure that what you get 
// is the same as what you put in.
// XXX it would be better to verify the FieldVValues, too

TEST_F(DriverTypesFixture, PATData) {
  std::vector<PATData> data = MakeRandomPATData();

  auto fvv = DataToFieldVValues(data);
  std::vector<PATData> data_out = FieldVValuesToPATData(fvv);

  ComparePATData(data, data_out);
}

TEST_F(DriverTypesFixture, TATData) {
  std::vector<TATData> data = MakeRandomTATData();

  auto fvv = DataToFieldVValues(data);
  std::vector<TATData> data_out = FieldVValuesToTATData(fvv);

  CompareTATData(data, data_out);
}

TEST_F(DriverTypesFixture, AMData) {
  std::vector<AMData> data = MakeRandomAMData();

  auto fvv = DataToFieldVValues(data);
  std::vector<AMData> data_out = FieldVValuesToAMData(fvv);

  CompareAMData(data, data_out);
}

TEST_F(DriverTypesFixture, MMData) {
  std::vector<MMData> data = MakeRandomMMData();

  auto fvv = DataToFieldVValues(data);
  std::vector<MMData> data_out = FieldVValuesToMMData(fvv);

  CompareMMData(data, data_out);
}
