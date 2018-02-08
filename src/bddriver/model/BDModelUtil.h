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
// basically just copy-pasted from Driver/Encoder/Decoder

////////////////////////////////////////
// downstream functions

/// Does inverse of Encoder byte-unpacking (looks like Decoder)
std::vector<uint32_t> FPGAInput(std::vector<EncOutput> inputs, const bdpars::BDPars* pars);

/// Does inverse of SendToEP's serialization (looks like RecvFromEP)
std::vector<BDWord> DeserializeEP(uint8_t code, const std::vector<uint32_t>& inputs, unsigned int D);

////////////////////////////////////////
// downstream functions

/// Does inverse of RecvFromEP's deserialization (looks like SendToEP)
std::vector<uint32_t> SerializeEP(const std::vector<BDWord>& inputs, unsigned int D);

/// Does inverse of Decoder byte-packing (looks like Encoder)
std::vector<DecInput> FPGAOutput(std::vector<uint32_t> inputs, const bdpars::BDPars* pars);

}  // bdmodel
}  // bddriver
}  // pystorm

#endif
