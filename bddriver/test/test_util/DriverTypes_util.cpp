#include "DriverTypes_util.h"
#include "DriverTypes.h"
#include "BDPars.h"
#include "gtest/gtest.h"

#include <limits>
#include <random>
#include <cstdint>

using namespace pystorm;
using namespace bddriver;

std::vector<PATData> MakeRandomPATData(unsigned int N) 
{
  using namespace bdpars;
  BDPars pars = BDPars();

  auto generator = std::default_random_engine(0);
  auto uint_distribution = std::uniform_int_distribution<unsigned int>(0, std::numeric_limits<unsigned int>::max());

  std::vector<PATData> data;
  data.resize(N);
  for (unsigned int i = 0; i < N; i++) {
    data[i] = {
      uint_distribution(generator) % (1 << pars.WordFieldWidth(PAT, AM_ADDRESS)),
      uint_distribution(generator) % (1 << pars.WordFieldWidth(PAT, MM_ADDRESS_LO)),
      uint_distribution(generator) % (1 << pars.WordFieldWidth(PAT, MM_ADDRESS_HI))
    };
  }
  return data;
}

std::vector<TATData> MakeRandomTATData(unsigned int N) 
{
  using namespace bdpars;
  BDPars pars = BDPars();

  auto generator = std::default_random_engine(0);
  auto uint_distribution = std::uniform_int_distribution<unsigned int>(0, std::numeric_limits<unsigned int>::max());

  std::vector<TATData> data;
  data.resize(N);
  for (unsigned int i = 0; i < N; i++) {
    unsigned int stop = uint_distribution(generator) % 2;
    unsigned int type = uint_distribution(generator) % 3;
    if (type == 0) {
      data[i] = {
        stop, 
        type,
        uint_distribution(generator) % (1 << pars.WordFieldWidth(TAT0, AM_ADDRESS, 0)), 
        uint_distribution(generator) % (1 << pars.WordFieldWidth(TAT0, MM_ADDRESS, 0)), 
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
        uint_distribution(generator) % (1 << pars.WordFieldWidth(TAT0, SYNAPSE_ADDRESS_0, 1)), 
        sign0,
        uint_distribution(generator) % (1 << pars.WordFieldWidth(TAT0, SYNAPSE_ADDRESS_1, 1)), 
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
        uint_distribution(generator) % (1 << pars.WordFieldWidth(TAT0, TAG, 2)), 
        uint_distribution(generator) % (1 << pars.WordFieldWidth(TAT0, GLOBAL_ROUTE, 2))
      };
    } else {
      assert(false);
    }
  }
  return data;
}

std::vector<AMData> MakeRandomAMData(unsigned int N) 
{
  using namespace bdpars;
  BDPars pars = BDPars();

  auto generator = std::default_random_engine(0);
  auto uint_distribution = std::uniform_int_distribution<unsigned int>(0, std::numeric_limits<unsigned int>::max());

  std::vector<AMData> data;
  data.resize(N);
  for (unsigned int i = 0; i < N; i++) {
    unsigned int stop = uint_distribution(generator) % 2;
    data[i] = {
      uint_distribution(generator) % (1 << pars.WordFieldWidth(AM, THRESHOLD)), 
      stop,
      uint_distribution(generator) % (1 << pars.WordFieldWidth(AM, NEXT_ADDRESS))
    };
  }
  return data;
}

std::vector<MMData> MakeRandomMMData(unsigned int N) 
{
  using namespace bdpars;
  BDPars pars = BDPars();

  auto generator = std::default_random_engine(0);
  auto uint_distribution = std::uniform_int_distribution<unsigned int>(0, std::numeric_limits<unsigned int>::max());

  std::vector<MMData> data;
  data.resize(N);
  for (unsigned int i = 0; i < N; i++) {
    data[i] = uint_distribution(generator) % (1 << pars.WordFieldWidth(MM, WEIGHT));
  };
  return data;
}

std::vector<SynSpike> MakeRandomSynSpikesSameCoreId(unsigned int N, unsigned int core_id)
{
  std::vector<unsigned int> core_ids(N, core_id);

  auto generator = std::default_random_engine(0);
  auto uint_distribution = std::uniform_int_distribution<unsigned int>(0, std::numeric_limits<unsigned int>::max());
  std::vector<unsigned int> times;
  for (unsigned int i = 0; i < N; i++) {
    times.push_back(uint_distribution(generator));
  }

  return MakeRandomSynSpikes(N, times, core_ids);
}

std::vector<SynSpike> MakeRandomSynSpikes(unsigned int N, const std::vector<unsigned int> & time, const std::vector<unsigned int> & core_id)
{
  using namespace bdpars;
  BDPars pars = BDPars();

  auto generator = std::default_random_engine(0);
  auto uint_distribution = std::uniform_int_distribution<unsigned int>(0, std::numeric_limits<unsigned int>::max());

  std::vector<SynSpike> data;
  data.resize(N);
  for (unsigned int i = 0; i < N; i++) {
    int sign = 2*(uint_distribution(generator) % 2) - 1;
    data[i] = {
      time[i],
      core_id[i],
      uint_distribution(generator) % (1 << pars.WordFieldWidth(INPUT_SPIKES, SYNAPSE_ADDRESS)),
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
