#ifndef BDMODELUTIL_H
#define BDMODELUTIL_H

#include <array>
#include <vector>

#include "common/BDState.h"
#include "common/BDPars.h"
#include "common/DriverTypes.h"

namespace pystorm {
namespace bddriver {
namespace bdmodel {

// "inverse" driver functions: models of what the FPGA + BD hardware does

////////////////////////////////////////
// downstream functions

/// Packs byte stream, will do other stuff at some point
std::vector<uint32_t> FPGAInput(std::vector<EncOutput> inputs, const bdpars::BDPars* pars);

/// Does deserialization
/// return types same as DeserializeWords
std::pair<std::vector<uint64_t>, std::vector<uint32_t>> 
  DeserializeHorn(const std::vector<uint32_t>& inputs, bdpars::BDHornEP leaf_id, const bdpars::BDPars* bd_pars);

/// Does deserialization for all BD leaves
/// returns a pair of dictionaries keyed by ep code
std::pair<std::unordered_map<uint8_t, std::vector<uint64_t>>, 
          std::unordered_map<uint8_t, std::vector<uint32_t>>>
    DeserializeAllCodes(const std::unordered_map<uint8_t, std::vector<uint32_t>>& inputs, const bdpars::BDPars* bd_pars);

////////////////////////////////////////
// downstream functions

/// Does serialization, returns pairs of {serialized words chunks, word chunk widths}
std::pair<std::vector<uint64_t>, unsigned int> SerializeFunnel(
    const std::vector<uint64_t>& inputs, bdpars::BDFunnelEP leaf_id, const bdpars::BDPars* bd_pars);

/// Does serialization for all Funnel leaves
std::unordered_map<uint8_t, std::pair<std::vector<uint64_t>, unsigned int>> 
    SerializeAllCodes(const std::unordered_map<uint8_t, std::vector<uint64_t>>& inputs, const bdpars::BDPars* bd_pars);

/// Unpacks byte stream, will do other stuff eventually
std::vector<DecInput> FPGAOutput(std::vector<uint32_t> inputs, const bdpars::BDPars* pars);

}  // bdmodel
}  // bddriver
}  // pystorm

#endif
