#ifndef DRIVERTYPES_UTIL_H
#define DRIVERTYPES_UTIL_H

#include "BDPars.h"
#include <vector>

using namespace pystorm;
using namespace bddriver;

std::vector<BDWord> MakeRandomBDWords(unsigned int bit_len, unsigned int N);

inline std::vector<BDWord> MakeRandomPATData(unsigned int N) {
  return MakeRandomBDWords(PATWord::width, N);
}

inline std::vector<BDWord> MakeRandomTATData(unsigned int N) {
  // this is the only special case. There's an illegal bit (can't allow FIXED=3)
  std::vector<BDWord> words = MakeRandomBDWords(TATAccWord::width, N);
  // turn all the undefined FIXED=3 words into something else
  for (unsigned int i = 0; i < N; i++) {
    if (words[i].At<TATAccWord>(TATAccWord::FIXED_0) == 3) {
      words[i] = BDWord(0); // XXX just zero it out, I guess, it's a valid word
    }
  }
  return words;
}

inline std::vector<BDWord> MakeRandomAMData(unsigned int N) {
  return MakeRandomBDWords(AMWord::width, N);
}

inline std::vector<BDWord> MakeRandomMMData(unsigned int N) {
  return MakeRandomBDWords(MMWord::width, N);
}

inline std::vector<BDWord> MakeRandomSynSpikes(unsigned int N) {
  return MakeRandomBDWords(InputSpike::width, N);
}

inline std::vector<BDWord> MakeRandomNrnSpikes(unsigned int N) {
  return MakeRandomBDWords(OutputSpike::width, N);
}

inline std::vector<BDWord> MakeRandomInputTags(unsigned int N) {
  return MakeRandomBDWords(InputTag::width, N);
}

inline std::vector<BDWord> MakeRandomPreFIFOTags(unsigned int N) {
  return MakeRandomBDWords(PreFIFOTag::width, N);
}

inline std::vector<BDWord> MakeRandomPostFIFOTags(unsigned int N) {
  return MakeRandomBDWords(PostFIFOTag::width, N);
}

#endif
