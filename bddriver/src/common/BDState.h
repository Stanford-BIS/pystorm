#ifndef BDSTATE_H
#define BDSTATE_H

#include <string>
#include <vector>
#include <unordered_map>

#include "BDPars.h"

namespace pystorm {
namespace bddriver {

class BDState {
  // keeps track of currently set register values, toggle states, memory values, etc.
  public:
    BDState(BDPars * pars);
    ~BDState();

    void SetRegister(const std::string & reg_name, uint64_t new_val);
    uint64_t GetRegister(const std::string & reg_name) const;

    void SetMemory(const std::string & mem_name, unsigned int start_addr, const std::vector<uint64_t> & new_vals);
    std::vector<uint64_t> GetMemory(const std::string & mem_name, unsigned int start_addr, unsigned int num_vals) const;

  private:
    const BDPars * pars_;
    std::unordered_map<std::string, uint64_t> reg_vals_;
    std::unordered_map<std::string, std::vector<uint64_t> > mem_vals_;

};

} // bddriver
} // pystorm

#endif
