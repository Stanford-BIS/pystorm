#include "DriverTypes_util.h"
#include "DriverTypes.h"
#include "gtest/gtest.h"

#include <limits>
#include <random>
#include <cstdint>

using namespace pystorm;
using namespace bddriver;

std::vector<PATData> MakeRandomPATData(unsigned int N) 
{
  auto generator = std::default_random_engine(0);
  auto uint_distribution = std::uniform_int_distribution<unsigned int>(0, std::numeric_limits<unsigned int>::max());

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

std::vector<TATData> MakeRandomTATData(unsigned int N) 
{
  auto generator = std::default_random_engine(0);
  auto uint_distribution = std::uniform_int_distribution<unsigned int>(0, std::numeric_limits<unsigned int>::max());

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

std::vector<AMData> MakeRandomAMData(unsigned int N) 
{
  auto generator = std::default_random_engine(0);
  auto uint_distribution = std::uniform_int_distribution<unsigned int>(0, std::numeric_limits<unsigned int>::max());

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

std::vector<MMData> MakeRandomMMData(unsigned int N) 
{
  auto generator = std::default_random_engine(0);
  auto uint_distribution = std::uniform_int_distribution<unsigned int>(0, std::numeric_limits<unsigned int>::max());

  std::vector<MMData> data;
  data.reserve(N);
  for (unsigned int i = 0; i < N; i++) {
    data[i] = uint_distribution(generator);
  };
  return data;
}

std::vector<SynSpike> MakeRandomSynSpikesSameCoreId(unsigned int N, unsigned int core_id)
{
  auto generator = std::default_random_engine(0);
  auto uint_distribution = std::uniform_int_distribution<unsigned int>(0, std::numeric_limits<unsigned int>::max());

  std::vector<SynSpike> data;
  data.reserve(N);
  for (unsigned int i = 0; i < N; i++) {
    int sign = 2*(uint_distribution(generator) % 2) - 1;
    data[i] = {
      uint_distribution(generator),
      core_id,
      uint_distribution(generator),
      sign
    };
  };
  return data;

}

// XXX Deprecated? should be able to say ASSERT_EQ(a, b) now that operator== is defined for these

void ComparePATData(const std::vector<PATData> & a, const std::vector<PATData> & b)
{
  ASSERT_EQ(a, b);
}

void CompareTATData(const std::vector<TATData> & a, const std::vector<TATData> & b)
{
  ASSERT_EQ(a, b);
}

void CompareAMData(const std::vector<AMData> & a, const std::vector<AMData> & b)
{
  ASSERT_EQ(a, b);
}

void CompareMMData(const std::vector<MMData> & a, const std::vector<MMData> & b)
{
  ASSERT_EQ(a, b);
}
