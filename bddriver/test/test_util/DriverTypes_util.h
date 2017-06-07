#ifndef DRIVERTYPES_UTIL_H
#define DRIVERTYPES_UTIL_H

#include "DriverTypes.h"
#include <vector>

using namespace pystorm;
using namespace bddriver;

std::vector<PATData> MakeRandomPATData(unsigned int N);
std::vector<TATData> MakeRandomTATData(unsigned int N);
std::vector<AMData> MakeRandomAMData(unsigned int N);
std::vector<MMData> MakeRandomMMData(unsigned int N);

std::vector<SynSpike> MakeRandomSynSpikesSameCoreId(unsigned int N, unsigned int core_id);
std::vector<SynSpike> MakeRandomSynSpikes(unsigned int N, const std::vector<unsigned int> & time, const std::vector<unsigned int> & core_id);
std::vector<Tag> MakeRandomTags(unsigned int N, const std::vector<unsigned int>& time, const std::vector<unsigned int>& core_id);

void ComparePATData(const std::vector<PATData> & a, const std::vector<PATData> & b);
void CompareTATData(const std::vector<TATData> & a, const std::vector<TATData> & b);
void CompareAMData(const std::vector<AMData> & a, const std::vector<AMData> & b);
void CompareMMData(const std::vector<MMData> & a, const std::vector<MMData> & b);

#endif
