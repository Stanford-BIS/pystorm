#ifndef BDPARS_H
#define BDPARS_H

#include <string>
#include <vector>
#include <unordered_map>

namespace pystorm {
namespace bddriver {

// this COULD probably be a struct, but I like it as a class for protection: it's essentially read-only

typedef std::pair<uint32_t, unsigned int> FHRoute;

class BDPars {
  public:
    // init from yaml file describing chip pars
    BDPars();

    // funnel/horn queries
    inline FHRoute HornRoute(unsigned int horn_idx) const { return horn_routes_[horn_idx]; }
    inline FHRoute FunnelRoute(unsigned int funnel_idx) const { return funnel_routes_[funnel_idx]; }

    inline FHRoute HornRoute(const std::string& leaf) const { return HornRoute(HornIdx(leaf)); }
    inline FHRoute FunnelRoute(const std::string& leaf) const { return FunnelRoute(FunnelIdx(leaf)); }

    inline unsigned int HornIdx(const std::string& leaf) const { return horn_leaf_name_to_idx_.at(leaf); }
    inline unsigned int FunnelIdx(const std::string& leaf) const { return funnel_leaf_name_to_idx_.at(leaf); }

    // useful if you need to iterate through routing table, returns const ptr
    inline const std::vector<FHRoute> * HornRoutes() const { return &horn_routes_; }
    inline const std::vector<FHRoute> * FunnelRoutes() const { return &funnel_routes_; }

    // field width queries
    inline unsigned int Width(const std::string& field) const { return widths_.at(field); }

  private:
    unsigned int in_bits;
    
    // map between leaf name and index, used when enqueuing/dequeuing words
    std::unordered_map<std::string, unsigned int> horn_leaf_name_to_idx_;
    std::unordered_map<std::string, unsigned int> funnel_leaf_name_to_idx_;

    // funnel/horn route tables are direct-mapped
    std::vector<FHRoute> horn_routes_;
    std::vector<FHRoute> funnel_routes_;

    // various bit widths used in the HW
    std::unordered_map<std::string, unsigned int> widths_;

};

} // bddriver
} // pystorm

#endif
