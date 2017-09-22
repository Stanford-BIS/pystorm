#ifndef DRIVERTYPES_UTIL_H
#define DRIVERTYPES_UTIL_H

#include "BDPars.h"
#include "BDWord.h"
#include <vector>

using namespace pystorm;
using namespace bddriver;

// meant to be called with T::FIELDCOUNT as the arg
template <class T>
unsigned int GetWidth(T count) {
  unsigned int w = 0;
  for (unsigned int i = 0; i < static_cast<unsigned int>(count); i++) {
    w += FieldWidth(static_cast<T>(i));
  }
  return w;
}

std::vector<BDWord> MakeRandomBDWords(unsigned int bit_len, unsigned int N);

inline std::vector<BDWord> MakeRandomPATData(unsigned int N) {
  return MakeRandomBDWords(GetWidth(PATWord::FIELDCOUNT), N);
}

inline std::vector<BDWord> MakeRandomTATData(unsigned int N) {
  // this is the only special case. There's an illegal bit (can't allow FIXED=3)
  std::vector<BDWord> words = MakeRandomBDWords(GetWidth(TATAccWord::FIELDCOUNT), N);
  // turn all the undefined FIXED=3 words into something else
  for (unsigned int i = 0; i < N; i++) {
    if (GetField(words[i], TATAccWord::FIXED_0) == 3) {
      words[i] = BDWord(0); // XXX just zero it out, I guess, it's a valid word
    }
  }
  return words;
}


inline std::vector<BDWord> MakeRandomAMData(unsigned int N) {
  return MakeRandomBDWords(GetWidth(AMWord::FIELDCOUNT), N);
}

inline std::vector<BDWord> MakeRandomMMData(unsigned int N) {
  return MakeRandomBDWords(GetWidth(MMWord::FIELDCOUNT), N);
}

inline std::vector<BDWord> MakeRandomSynSpikes(unsigned int N) {
  return MakeRandomBDWords(GetWidth(InputSpike::FIELDCOUNT), N);
}

inline std::vector<BDWord> MakeRandomNrnSpikes(unsigned int N) {
  return MakeRandomBDWords(GetWidth(OutputSpike::FIELDCOUNT), N);
}

inline std::vector<BDWord> MakeRandomInputTags(unsigned int N) {
  return MakeRandomBDWords(GetWidth(InputTag::FIELDCOUNT), N);
}

inline std::vector<BDWord> MakeRandomPreFIFOTags(unsigned int N) {
  return MakeRandomBDWords(GetWidth(PreFIFOTag::FIELDCOUNT), N);
}

inline std::vector<BDWord> MakeRandomPostFIFOTags(unsigned int N) {
  return MakeRandomBDWords(GetWidth(PostFIFOTag::FIELDCOUNT), N);
}

#endif
