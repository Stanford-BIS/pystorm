#ifndef BDMODELUTIL_H
#define BDMODELUTIL_H

#include <array>

#include "common/BDState.h"
#include "common/BDPars.h"
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
std::array<std::vector<uint32_t>, bdpars::BDEndPointIdCount> Horn(const std::vector<uint32_t>& inputs, const bdpars::BDPars* pars);

/// Does deserialization
std::pair<std::vector<uint32_t>, std::vector<uint32_t> > 
  DeserializeHorn(const std::vector<uint32_t>& inputs, bdpars::BDEndPointId leaf_id, const bdpars::BDPars* bd_pars);

/// Does deserialization for all horn leaves
std::pair<std::array<std::vector<uint32_t>, bdpars::BDEndPointIdCount>, 
          std::array<std::vector<uint32_t>, bdpars::BDEndPointIdCount > > 
    DeserializeAllHornLeaves(const std::array<std::vector<uint32_t>, bdpars::BDEndPointIdCount>& inputs, const bdpars::BDPars* bd_pars);

////////////////////////////////////////
// downstream functions

/// Does serialization, returns pairs of {serialized words chunks, word chunk widths}
std::pair<std::vector<uint32_t>, unsigned int> SerializeFunnel(
    const std::vector<uint32_t>& inputs, bdpars::BDStartPointId leaf_id, const bdpars::BDPars* bd_pars);

/// Does serialization for all Funnel leaves
std::array<std::pair<std::vector<uint32_t>, unsigned int>, bdpars::BDStartPointIdCount> 
    SerializeAllFunnelLeaves(const std::array<std::vector<uint32_t>, bdpars::BDStartPointIdCount>& inputs, const bdpars::BDPars* bd_pars);

/// Does funnel operation
std::vector<uint32_t> Funnel(const std::array<std::pair<std::vector<uint32_t>, unsigned int>, bdpars::BDStartPointIdCount>& inputs, const bdpars::BDPars* pars);

/// Unpacks byte stream, will do other stuff eventually
std::vector<DecInput> FPGAOutput(std::vector<uint32_t> inputs, const bdpars::BDPars* pars);

}  // bdmodel
}  // bddriver
}  // pystorm

#endif
