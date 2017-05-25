#ifndef BDMODELUTIL_H
#define BDMODELUTIL_H

#include "common/DriverTypes.h"
#include "common/BDState.h"
#include "Driver.h"

namespace pystorm {
namespace bddriver {
namespace bdmodel {

// "inverse" driver functions: models of what the BD hardware does

////////////////////////////////////////
// downstream functions

/// Packs byte stream, will do other stuff at some point
std::vector<uint32_t> FPGAInput(std::vector<EncOutput> inputs, const bdpars::BDPars * pars);

/// Does horn operation
std::vector<std::vector<uint32_t> > Horn(const std::vector<uint32_t> & inputs, const bdpars::BDPars * pars);

/// Does deserialization
std::vector<uint64_t> Deserialize(bdpars::HornLeafId leaf_id, const std::vector<uint32_t> & inputs);
std::vector<std::vector<uint64_t> > DeserializeAllHornLeaves(const std::vector<std::vector<uint32_t> > & inputs, const bdpars::BDPars * bd_pars);

/// Interprets payloads sent to a particular leaf as a vector of FieldVValues
std::vector<FieldVValues> ParseHornLeaf(const std::vector<uint64_t> inputs, bdpars::HornLeafId leaf_id, const bdpars::BDPars * bd_pars);

////////////////////////////////////////
// helper functions that were general enough something else might want to use them

FieldValues ParsePATWord(uint64_t input, const bdpars::BDPars * bd_pars);
FieldValues ParseTATWord(uint64_t input, const bdpars::BDPars * bd_pars);
FieldValues ParseAMMMWord(uint64_t input, const bdpars::BDPars * bd_pars);
FieldValues ParseAMWord(FieldValues input, const bdpars::BDPars * bd_pars);
FieldValues ParseMMWord(FieldValues input, const bdpars::BDPars * bd_pars);

/// puts one FVV inside another
std::pair<unsigned int, FieldValues> DeencapsulateWord(const FieldValues & current, bdpars::WordFieldId descend_into, const std::vector<const bdpars::WordStructure *> & words_to_try);

// XXX there are additional helper functions in BDModelUtil.cpp

} // bdmodel
} // bddriver
} // pystorm

#endif
