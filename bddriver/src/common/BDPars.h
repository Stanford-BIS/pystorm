#ifndef BDPARS_H
#define BDPARS_H

#include <string>
#include <unordered_map>

#include "Binary.h"

namespace pystorm {
namespace bddriver {

class BDPars {
  public:
    // init from yaml file describing chip pars
    BDPars(std::string bd_yaml);

    // funnel/horn queries
    inline const Binary * HornRoute(const std::string& leaf) const { return &horn_routes_.at(leaf); }
    inline const Binary * FunnelRoute(const std::string& leaf) const { return &funnel_routes_.at(leaf); }

    // if you need to iterate through them, say
    inline const std::unordered_map<std::string, Binary> * HornRoutes() const { return &horn_routes_; }
    inline const std::unordered_map<std::string, Binary> * FunnelRoutes() const { return &funnel_routes_; }

    // field width queries
    inline uint8_t Width(const std::string& field) const { return widths_.at(field); }

  private:
    unsigned int in_bits;

    // funnel/horn route tables
    std::unordered_map<std::string, Binary> horn_routes_;
    std::unordered_map<std::string, Binary> funnel_routes_;
    // various bit widths used in the HW
    std::unordered_map<std::string, uint8_t> widths_;

};

} // bddriver
} // pystorm

#endif
