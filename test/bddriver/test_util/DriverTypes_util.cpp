#include "DriverTypes_util.h"
#include "gtest/gtest.h"

#include <cstdint>
#include <limits>
#include <random>

using namespace pystorm;
using namespace bddriver;

std::vector<BDWord> MakeRandomBDWords(unsigned int bit_len, unsigned int N) {
  auto generator         = std::default_random_engine(0);
  auto uint_distribution = std::uniform_int_distribution<uint64_t>(0, std::numeric_limits<uint64_t>::max());
  
  uint64_t max_val = (static_cast<uint64_t>(1) << bit_len) - 1;


  std::vector<BDWord> words;
  for (unsigned int i = 0; i < N; i++) {
    words.push_back(uint_distribution(generator) % max_val);
  }
  return words;
}

