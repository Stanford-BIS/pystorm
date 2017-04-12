#include "BDState.h"

#include <vector>

#include "BDPars.h"

namespace pystorm {
namespace bddriver {

BDState::BDState(BDPars * pars)
{
  // use pars to initialize all fields
}

void BDState::SetRegister(const std::string & reg_name, uint64_t new_val) 
{
}

void BDState::SetMemory(const std::string & mem_name, unsigned int start_addr, const std::vector<uint64_t> & new_vals)
{
}

} // bddriver
} // pystorm
