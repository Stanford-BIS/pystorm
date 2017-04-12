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
    void SetMemory(const std::string & mem_name, unsigned int start_addr, const std::vector<uint64_t> & new_vals);

  private:
    unsigned int in_bits;

    std::vector<std::string> register_names_;
    std::vector<std::string> memory_names_;

    std::unordered_map<std::string, uint64_t> register_vals_;
    std::unordered_map<std::string, uint64_t *> memory_vals_;

};

} // bddriver
} // pystorm

#endif
