#ifndef BDPARS_H
#define BDPARS_H

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include <iostream>
using std::cout;
using std::endl;

namespace pystorm {
namespace bddriver {

// originally all the dictionary keys were strings
// using enums is better:
// the set of possible keys is fixed
// typos will be caught by compiler
// dictionary lookup is possibly faster

// XXX DO NOT ATTEMPT TO ASSIGN ENUM VALUES WITHOUT UNDERSTANDING horn_routes_ and funnel_routes_
// the enum values are used to index these vectors

enum HornLeafId {
  NeuronInject,
  RI,
  PROG_AMMM,
  PROG_PAT,
  PROG_TAT0,
  PROG_TAT1,
  INIT_FIFO_DCT,
  INIT_FIFO_HT,
  TOGGLE_PRE_FIFO_leaf,
  TOGGLE_POST_FIFO0_leaf,
  TOGGLE_POST_FIFO1_leaf,
  NeuronDumpToggle_leaf,
  NeuronConfig,
  DAC0_leaf,
  DAC1_leaf,
  DAC2_leaf,
  DAC3_leaf,
  DAC4_leaf,
  DAC5_leaf,
  DAC6_leaf,
  DAC7_leaf,
  DAC8_leaf,
  DAC9_leaf,
  DAC10_leaf,
  DAC11_leaf,
  DAC12_leaf,
  ADC_leaf,
  DELAY0_leaf,
  DELAY1_leaf,
  DELAY2_leaf,
  DELAY3_leaf,
  DELAY4_leaf,
  DELAY5_leaf,
  DELAY6_leaf
};

enum FunnelLeafId {
  RO_ACC,
  RO_TAT,
  NRNI,
  DUMP_AM,
  DUMP_MM,
  DUMP_PAT,
  DUMP_TAT0,
  DUMP_TAT1,
  DUMP_PRE_FIFO,
  DUMP_POST_FIFO0,
  DUMP_POST_FIFO1,
  OVFLW0,
  OVFLW1
};

enum RegId {
  TOGGLE_PRE_FIFO,
  TOGGLE_POST_FIFO0,
  TOGGLE_POST_FIFO1,
  NeuronDumpToggle,
  DAC0,
  DAC1,
  DAC2,
  DAC3,
  DAC4,
  DAC5,
  DAC6,
  DAC7,
  DAC8,
  DAC9,
  DAC10,
  DAC11,
  DAC12,
  ADC,
  DELAY0,
  DELAY1,
  DELAY2,
  DELAY3,
  DELAY4,
  DELAY5,
  DELAY6
};

enum MemId {
  AM,
  MM,
  TAT0,
  TAT1,
  PAT
};

enum MemWordId {
  // PAT words
  PAT_write,
  PAT_read,
  // TAT words
  TAT_set_address,
  TAT_write_increment,
  TAT_read_increment,
  // MM words
  MM_set_address,
  MM_write_increment,
  MM_read_increment,
  // AM words
  AM_set_address,
  AM_read_write,
  AM_increment,
  // AM/MM encapsulation
  AM_encapsulation,
  MM_encapsulation
};

enum WordFieldId {
  // fields with special meaning
  FIXED_0,
  FIXED_1,
  FIXED_2,
  FIXED_3, // if you need a higher FIXED value, need to add it here and encode meaning (or use multiple fields)
  unused,
  // common fields
  tag,
  global_route,
  count,
  address,
  data,
  payload,
  stop,
  // registers
  traffic_enable,
  dump_enable,
  DAC_to_ADC_conn,
  DAC_value,
  read_delay,
  write_delay,
  ADC_small_large_current_0,
  ADC_small_large_current_1,
  ADC_output_enable,
  // AM data
  value,
  threshold,
  next_address,
  // MM data
  weight,
  // PAT data
  AM_address,
  MM_address_lo,
  MM_address_hi,
  // TAT data
  MM_address,
  synapse_address_0,
  synapse_sign_0,
  synapse_address_1,
  synapse_sign_1,
  // inputs and outputs
  synapse_address,
  synapse_sign,
  neuron_address,
};

// XXX I don't like this. Identifiers for things that don't fit in other categories
enum MiscId {
  BD_input,
  BD_output
};

// XXX TODO fill me in, meant to support DACSignalIdToDACRegisterId
enum DACSignalId {
  PlaceholderSignalName
};

typedef std::pair<uint32_t, unsigned int> FHRoute; // route val, route len
typedef std::map<WordFieldId, unsigned int> WordStructure; // field name, field width, in order lsb -> msb

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
  HornLeafId prog_leaf;
  FunnelLeafId dump_leaf;
};

struct RegInfo {
  WordStructure word_structure;
  HornLeafId prog_leaf;
};

class BDPars {
  public:
    BDPars();

    /////////////////////////////////////
    // funnel/horn queries
    
    inline FHRoute HornRoute(HornLeafId leaf)     const { return horn_routes_[leaf]; }
    inline FHRoute HornRoute(unsigned int leaf)   const { return horn_routes_[leaf]; }
    inline FHRoute FunnelRoute(FunnelLeafId leaf) const { return funnel_routes_[leaf]; }
    inline FHRoute FunnelRoute(unsigned int leaf) const { return funnel_routes_[leaf]; }

    inline unsigned int HornIdx(HornLeafId leaf)     const { return static_cast<unsigned int>(leaf); }
    inline unsigned int FunnelIdx(FunnelLeafId leaf) const { return static_cast<unsigned int>(leaf); }

    // useful if you need to iterate through routing table, returns const ptr
    inline const std::vector<FHRoute> * HornRoutes()   const { return &horn_routes_; }
    inline const std::vector<FHRoute> * FunnelRoutes() const { return &funnel_routes_; }

    /////////////////////////////////////
    // field width queries
    
    inline unsigned int Width(HornLeafId object)   const { return horn_.at(object).data_width; }
    inline unsigned int Width(FunnelLeafId object) const { return funnel_.at(object).data_width; }
    inline unsigned int Width(MiscId object)       const { return misc_pars_.at(object); }

    inline const WordStructure * Word(MemId object, unsigned int subtype_idx=0) const { return &(mem_.at(object).word_structures.at(subtype_idx)); }
    inline const WordStructure * Word(MemWordId object)    const { return &(mem_prog_words_.at(object)); }
    inline const WordStructure * Word(RegId object)        const { return &(reg_.at(object).word_structure); }
    inline const WordStructure * Word(HornLeafId object)   const { return &(input_.at(object)); }
    inline const WordStructure * Word(FunnelLeafId object) const { return &(output_.at(object)); }

    inline HornLeafId ProgHornId(MemId object) const { return mem_.at(object).prog_leaf; }
    inline HornLeafId ProgHornId(RegId object) const { return reg_.at(object).prog_leaf; }

    inline FunnelLeafId DumpFunnelId(MemId object) const { return mem_.at(object).dump_leaf; }

    inline unsigned int Size(const MemId object) const { return mem_sizes_.at(object); }

    RegId DACSignalIdToDACRegisterId(DACSignalId id) const;

    /////////////////////////////////////
    // bit-packing helper functions

  private:
    std::unordered_map<MiscId, unsigned int> misc_pars_;

    /// inputs to BD that aren't a register or memory programming word
    std::unordered_map<HornLeafId, WordStructure> input_;
    /// outputs from BD that aren't a memory dump word
    std::unordered_map<FunnelLeafId, WordStructure> output_;
    
    /// horn description
    std::unordered_map<HornLeafId, LeafInfo> horn_;
    /// funnel description
    std::unordered_map<FunnelLeafId, LeafInfo> funnel_;

    /// memory descriptions (data field packing + misc info)
    std::unordered_map<MemId, MemInfo> mem_;

    /// memory programming words
    std::unordered_map<MemWordId, WordStructure> mem_prog_words_;

    /// register descriptions (data field packing + misc info)
    std::unordered_map<RegId, RegInfo> reg_;

    // memory stuff
    std::unordered_map<MemId, unsigned int> mem_sizes_;
    std::unordered_map<MemId, unsigned int> mem_word_widths_;
    
    // map between leaf name and index, used when enqueuing/dequeuing words
    std::unordered_map<HornLeafId, unsigned int> horn_leaf_name_to_idx_;
    std::unordered_map<FunnelLeafId, unsigned int> funnel_leaf_name_to_idx_;

    // funnel/horn route tables are direct-mapped
    std::vector<FHRoute> horn_routes_;
    std::vector<FHRoute> funnel_routes_;

};

} // bddriver
} // pystorm

#endif
