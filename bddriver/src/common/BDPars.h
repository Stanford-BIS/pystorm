#ifndef BDPARS_H
#define BDPARS_H

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

namespace pystorm {
namespace bddriver {

typedef std::pair<uint32_t, unsigned int> FHRoute; // route val, route len
typedef std::map<std::string, unsigned int> WordStructure; // field name, field width

struct LeafInfo {
  /// Information describing one funnel/horn leaf in Braindrop
  uint32_t route_val;         ///< route to leaf
  unsigned int route_len;     ///< depth in funnel/horn tree (# bits in route_val)
  unsigned int data_width;    ///< width of data at leaf
  unsigned int serialization; ///< number of serializers/deserializers
  unsigned int chunk_width;   ///< width of data before/after any deserializers/serializers
  std::string description;    ///< brief description of leaf purpose
};

struct MemInfo {
  unsigned int size;
  std::vector<WordStructure> word_structures;
};

struct RegInfo {
  WordStructure word_structure;
};

class BDPars {
  public:
    BDPars();

    /////////////////////////////////////
    // funnel/horn queries
    
    inline FHRoute HornRoute(unsigned int horn_idx) const { return horn_routes_[horn_idx]; }
    inline FHRoute FunnelRoute(unsigned int funnel_idx) const { return funnel_routes_[funnel_idx]; }

    inline FHRoute HornRoute(const std::string& leaf) const { return HornRoute(HornIdx(leaf)); }
    inline FHRoute FunnelRoute(const std::string & leaf) const { return FunnelRoute(FunnelIdx(leaf)); }

    inline unsigned int HornIdx(const std::string & leaf) const { return horn_leaf_name_to_idx_.at(leaf); }
    inline unsigned int FunnelIdx(const std::string & leaf) const { return funnel_leaf_name_to_idx_.at(leaf); }

    // useful if you need to iterate through routing table, returns const ptr
    inline const std::vector<FHRoute> * HornRoutes() const { return &horn_routes_; }
    inline const std::vector<FHRoute> * FunnelRoutes() const { return &funnel_routes_; }

    /////////////////////////////////////
    // field width queries
    
    unsigned int Width(const std::string & object) const;

    const std::string * DACSignalNameToDACRegisterName(const std::string & signal_name);

    const WordStructure * Word(const std::string & object, unsigned int subtype_idx=0) const;

    /////////////////////////////////////
    // bit-packing helper functions

  private:
    unsigned int io_input_width_;
    unsigned int io_output_width_;

    /// inputs to BD that aren't a register or memory programming word
    std::unordered_map<std::string, WordStructure> input_;
    /// outputs from BD that aren't a memory dump word
    std::unordered_map<std::string, WordStructure> output_;
    
    // map, not unordered_map, the order that things are inserted into this is the mapping to index numbers
    /// horn description
    std::map<std::string, LeafInfo> horn_;
    /// funnel description
    std::map<std::string, LeafInfo> funnel_;

    /// memory descriptions
    std::unordered_map<std::string, MemInfo> mem_;

    /// memory programming words
    std::unordered_map<std::string, WordStructure> mem_prog_words_;

    /// register descriptions
    std::unordered_map<std::string, RegInfo> reg_;

    std::vector<std::string> register_names_;


    // memory stuff
    std::unordered_map<std::string, unsigned int> mem_sizes_;
    std::unordered_map<std::string, unsigned int> mem_word_widths_;
    
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
