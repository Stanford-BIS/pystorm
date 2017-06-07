#ifndef BDMODELUTIL_H
#define BDMODELUTIL_H

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
std::pair<std::vector<uint64_t>, std::vector<uint32_t> > DeserializeHorn(
    const std::vector<uint32_t>& inputs, bdpars::HornLeafId leaf_id, const bdpars::BDPars* bd_pars);
std::pair<std::vector<std::vector<uint64_t> >, std::vector<std::vector<uint32_t> > > DeserializeAllHornLeaves(
    const std::vector<std::vector<uint32_t> >& inputs, const bdpars::BDPars* bd_pars);

////////////////////////////////////////
// downstream functions

/// Does serialization, returns pairs of {serialized words chunks, word chunk widths}
std::pair<std::vector<uint32_t>, unsigned int> SerializeFunnel(
    const std::vector<uint64_t>& inputs, bdpars::FunnelLeafId leaf_id, const bdpars::BDPars* bd_pars);
std::vector<std::pair<std::vector<uint32_t>, unsigned int> > SerializeAllFunnelLeaves(
    const std::vector<std::vector<uint64_t> >& inputs, const bdpars::BDPars* bd_pars);

/// Does funnel operation
std::vector<uint32_t> Funnel(const std::vector<std::pair<std::vector<uint32_t>, unsigned int> >& inputs, const bdpars::BDPars* pars);

/// Unpacks byte stream, will do other stuff eventually
std::vector<DecInput> FPGAOutput(std::vector<uint32_t> inputs, const bdpars::BDPars* pars);

}  // bdmodel
}  // bddriver
}  // pystorm

#endif
