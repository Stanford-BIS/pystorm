#include "BDState.h"

#include <vector>
#include <assert.h>

#include "BDPars.h"

namespace pystorm {
namespace bddriver {

BDState::BDState(BDPars * pars)
{
  pars_ = pars;
  // use pars to initialize all fields
}

void BDState::SetRegister(const std::string & reg_name, uint64_t new_val) 
{
  assert(reg_vals_.count(reg_name) == 1 && "tried to set unknown register");
  assert(new_val <= MaxVal(pars_->Width(reg_name)) && "trying to set register value which exceeds register bitwidth");
  reg_vals_[reg_name] = new_val;
}

uint64_t BDState::GetRegister(const std::string & reg_name) const
{
  assert(reg_vals_.count(reg_name) == 1 && "tried to get unknown register");
  return reg_vals_.at(reg_name);
}


void BDState::SetMemory(const std::string & mem_name, unsigned int start_addr, const std::vector<uint64_t> & new_vals)
{
  assert(mem_vals_.count(mem_name) == 1 && "tried to set unknown memory");
  assert(start_addr + new_vals.size() < mem_vals_.at(mem_name).size() && "trying to assign address larger than memory size");
  
  for (unsigned int i = 0; i < new_vals.size(); i++) {
    uint64_t new_val = new_vals[i];
    assert(new_val <= MaxVal(pars_->MemWordWidth(mem_name)) && "trying to set memory value which exceeds memory bitwidth");
    unsigned int addr = start_addr + i;
    mem_vals_[mem_name][addr] = new_val;
  }
}

std::vector<uint64_t> BDState::GetMemory(const std::string & mem_name, unsigned int start_addr, unsigned int num_vals) const
{
  assert(mem_vals_.count(mem_name) == 1 && "tried to get unknown memory");
  assert(start_addr + new_vals.size() < mem_vals_.at(mem_name).size() && "trying to get address larger than memory size");

  std::vector<uint64_t> retval;
  retval.resize(num_vals);

  for (unsigned int i = 0; i < num_vals; i++) {
    unsigned int addr = start_addr + i;
    retval[i] = mem_vals_.at(mem_name).at(addr);
  }
}
  

} // bddriver
} // pystorm
