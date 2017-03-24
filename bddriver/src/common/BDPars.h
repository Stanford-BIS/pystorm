#ifndef BDPARS_H
#define BDPARS_H

#include <string>
#include <unordered_map>

#include "bddriver/common/Binary.h"

namespace pystorm {
namespace bddriver {

class BDPars {
  public:
    // init from yaml file describing chip pars
    BDPars(std::string bd_yaml);

    // funnel/horn queries
    inline const Binary * LeafRoute(const std::string& leaf) const { return &leaf_routes_.at(leaf); }

    // field width queries
    inline uint8_t Width(const std::string& field) const { return widths_.at(field); }

  private:
    unsigned int num_cores;
    // funnel/horn route tables
    std::unordered_map<std::string, Binary> leaf_routes_;
    // various bit widths used in the HW
    std::unordered_map<std::string, uint8_t> widths_;

};

} // bddriver
} // pystorm

#endif
