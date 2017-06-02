#ifndef BDMODELUTIL_H
#define BDMODELUTIL_H

#include "Driver.h"
#include "common/BDState.h"
#include "common/DriverTypes.h"

namespace pystorm {
namespace bddriver {
namespace bdmodel {

// "inverse" driver functions: models of what the BD hardware does

////////////////////////////////////////
// downstream functions

/// Packs byte stream, will do other stuff at some point
std::vector<uint32_t> FPGAInput(std::vector<EncOutput> inputs, const bdpars::BDPars* pars);

/// Does horn operation
std::vector<std::vector<uint32_t> > Horn(const std::vector<uint32_t>& inputs, const bdpars::BDPars* pars);

/// Does deserialization
std::vector<uint64_t> Deserialize(bdpars::HornLeafId leaf_id, const std::vector<uint32_t>& inputs);
std::vector<std::vector<uint64_t> > DeserializeAllHornLeaves(
    const std::vector<std::vector<uint32_t> >& inputs, const bdpars::BDPars* bd_pars);

}  // bdmodel
}  // bddriver
}  // pystorm

#endif
