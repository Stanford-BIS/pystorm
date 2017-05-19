#ifndef BDPARS_H
#define BDPARS_H

#include <string>
#include <vector>
#include <map>

#include <iostream>
using std::cout;
using std::endl;

namespace pystorm {
namespace bddriver {

// originally all the dictionary keys were strings
// using enums is better:
// the set of possible keys is fixed, and is documented explicitly
// typos will be caught by compiler
// dictionary lookup is probably faster (can use vectors instead, actually!)

// XXX DO NOT ATTEMPT TO ASSIGN ENUM VALUES XXX
// it will mess a lot of stuff up!
// the enum values are used to index these vectors

namespace bdpars {

enum ComponentTypeId {
  REG,
  MEM,
  INPUT,
  OUTPUT,

  LastComponentTypeId = OUTPUT
};

enum RegId {
  TOGGLE_PRE_FIFO,
  TOGGLE_POST_FIFO0,
  TOGGLE_POST_FIFO1,
  NEURON_DUMP_TOGGLE,
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
  DELAY6,

  LastRegId = DELAY6
};

enum MemId {
  AM,
  MM,
  TAT0,
  TAT1,
  PAT,

  LastMemId = PAT
};

enum InputId {
  INPUT_TAGS,
  DCT_FIFO_INPUT_TAGS,
  HT_FIFO_RESET,
  TILE_SRAM_INPUTS,
  INPUT_SPIKES,

  LastInputId = INPUT_SPIKES
};

enum OutputId {
  PRE_FIFO_TAGS,
  POST_FIFO_TAGS0,
  POST_FIFO_TAGS1,
  OUTPUT_SPIKES,
  OVERFLOW_TAGS0,
  OVERFLOW_TAGS1,
  ACC_OUTPUT_TAGS,
  TAT_OUTPUT_TAGS,

  LastOutputId = TAT_OUTPUT_TAGS
};


// XXX TODO fill me in, meant to support DACSignalIdToDACRegisterId
enum DACSignalId {
  PLACEHOLDER_SIGNAL_NAME,

  LastDACSignalId = PLACEHOLDER_SIGNAL_NAME
};

enum HornLeafId {
  NEURON_INJECT,
  RI,
  PROG_AMMM,
  PROG_PAT,
  PROG_TAT0,
  PROG_TAT1,
  INIT_FIFO_DCT,
  INIT_FIFO_HT,
  TOGGLE_PRE_FIFO_LEAF,
  TOGGLE_POST_FIFO0_LEAF,
  TOGGLE_POST_FIFO1_LEAF,
  NEURON_DUMP_TOGGLE_LEAF,
  NEURON_CONFIG,
  DAC0_LEAF,
  DAC1_LEAF,
  DAC2_LEAF,
  DAC3_LEAF,
  DAC4_LEAF,
  DAC5_LEAF,
  DAC6_LEAF,
  DAC7_LEAF,
  DAC8_LEAF,
  DAC9_LEAF,
  DAC10_LEAF,
  DAC11_LEAF,
  DAC12_LEAF,
  ADC_LEAF,
  DELAY0_LEAF,
  DELAY1_LEAF,
  DELAY2_LEAF,
  DELAY3_LEAF,
  DELAY4_LEAF,
  DELAY5_LEAF,
  DELAY6_LEAF,

  LastHornLeafId = DELAY6_LEAF
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
  OVFLW1,

  LastFunnelLeafId = OVFLW1
};

enum MemWordId {
  // PAT words
  PAT_WRITE,
  PAT_READ,
  // TAT WORDS
  TAT_SET_ADDRESS,
  TAT_WRITE_INCREMENT,
  TAT_READ_INCREMENT,
  // MM WORDS
  MM_SET_ADDRESS,
  MM_WRITE_INCREMENT,
  MM_READ_INCREMENT,
  // AM WORDS
  AM_SET_ADDRESS,
  AM_READ_WRITE,
  AM_INCREMENT,
  // AM/MM ENCAPSULATION
  AM_ENCAPSULATION,
  MM_ENCAPSULATION,

  LastMemWordId = MM_ENCAPSULATION
};

enum WordFieldId {
  // fields with special meaning
  INVALID_FIELD, // only used when you want an enum to not match anything
  FIXED_0,
  FIXED_1,
  FIXED_2,
  FIXED_3, // if you need a higher FIXED value, need to add it here and encode meaning (or use multiple fields)
  UNUSED,
  // common fields
  TAG,
  GLOBAL_ROUTE,
  COUNT,
  ADDRESS,
  DATA,
  PAYLOAD,
  STOP,
  // registers
  TRAFFIC_ENABLE,
  DUMP_ENABLE,
  DAC_TO_ADC_CONN,
  DAC_VALUE,
  READ_DELAY,
  WRITE_DELAY,
  ADC_SMALL_LARGE_CURRENT_0,
  ADC_SMALL_LARGE_CURRENT_1,
  ADC_OUTPUT_ENABLE,
  // AM data
  ACCUMULATOR_VALUE,
  THRESHOLD,
  NEXT_ADDRESS,
  // MM data
  WEIGHT,
  // AMMM encapsulation
  AMMM_STOP,
  // PAT data
  AM_ADDRESS,
  MM_ADDRESS_LO,
  MM_ADDRESS_HI,
  // TAT data
  MM_ADDRESS,
  SYNAPSE_ADDRESS_0,
  SYNAPSE_SIGN_0,
  SYNAPSE_ADDRESS_1,
  SYNAPSE_SIGN_1,
  // inputs and outputs
  SYNAPSE_ADDRESS,
  SYNAPSE_SIGN,
  NEURON_ADDRESS,

  LastWordFieldId = NEURON_ADDRESS
};

enum MiscWidthId {
  BD_INPUT,
  BD_OUTPUT,

  LastMiscWidthId = BD_OUTPUT
};

typedef std::pair<uint32_t, unsigned int> FHRoute; // route val, route len
typedef std::vector<std::pair<bdpars::WordFieldId, unsigned int> > WordStructure; // field name, field width, in order lsb -> msb

struct LeafInfo {
  /// Information describing one funnel/horn leaf in Braindrop
  ComponentTypeId component_type;
  unsigned int component;
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
  HornLeafId leaf;
};

struct InputInfo {
  WordStructure word_structure;
  HornLeafId leaf;
};

struct OutputInfo {
  WordStructure word_structure;
  FunnelLeafId leaf;
};

/// BDPars holds all the nitty-gritty information about the BD hardware's parameters.
///
/// BDPars contains several vector data members containing structs, keyed by enums.
/// The enums refer to particular hardware elements or concepts, such as the name of a memory, 
/// register, or a particular type of programming word.
/// The structs contain relevant information about this hardware element or concept,
/// such as the size of the memory, or the WordStructure that describes the programming word type.
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

    // look up serialization
    inline unsigned int Serialization(HornLeafId leaf)   const { return funnel_.at(leaf).serialization; }
    inline unsigned int Serialization(FunnelLeafId leaf) const { return horn_.at(leaf).serialization; }

    // going from a component Id to the FH leaf it's associated with
    inline HornLeafId HornLeafIdFor(MemId object)   const { return mem_.at(object).prog_leaf; }
    inline HornLeafId HornLeafIdFor(RegId object)   const { return reg_.at(object).leaf; }
    inline HornLeafId HornLeafIdFor(InputId object) const { return input_.at(object).leaf; }

    inline FunnelLeafId FunnelLeafIdFor(MemId object)    const { return mem_.at(object).dump_leaf; }
    inline FunnelLeafId FunnelLeafIdFor(OutputId object) const { return output_.at(object).leaf; }

    // going from a FH leaf to the value of the enum associate with the type of the component it services
    inline ComponentTypeId ComponentTypeIdFor(HornLeafId leaf)   const { return horn_.at(leaf).component_type; }
    inline ComponentTypeId ComponentTypeIdFor(FunnelLeafId leaf) const { return funnel_.at(leaf).component_type; }

    // going from a FH leaf to the value of the enum associate with the component it services
    inline unsigned int ComponentIdxFor(HornLeafId leaf)   const { return horn_.at(leaf).component; }
    inline unsigned int ComponentIdxFor(FunnelLeafId leaf) const { return funnel_.at(leaf).component; }

    /////////////////////////////////////
    // field width queries
    
    inline unsigned int Width(HornLeafId object)   const { return horn_.at(object).data_width; }
    inline unsigned int Width(FunnelLeafId object) const { return funnel_.at(object).data_width; }
    inline unsigned int Width(MiscWidthId object)  const { return misc_widths_.at(object); }

    /////////////////////////////////////
    // Word structure queries
    
    inline const WordStructure * Word(MemId object, unsigned int subtype_idx=0) const { return &(mem_.at(object).word_structures.at(subtype_idx)); }
    inline const WordStructure * Word(MemWordId object) const { return &(mem_prog_words_.at(object)); }
    inline const WordStructure * Word(RegId object)     const { return &(reg_.at(object).word_structure); }
    inline const WordStructure * Word(InputId object)   const { return &(input_.at(object).word_structure); }
    inline const WordStructure * Word(OutputId object)  const { return &(output_.at(object).word_structure); }

    /////////////////////////////////////
    // misc

    inline unsigned int Size(const MemId object) const { return mem_.at(object).size; }

    RegId DACSignalIdToDACRegisterId(DACSignalId id) const;

    inline unsigned int NumReg() const { return reg_.size(); }
    inline unsigned int NumCores() const { return num_cores_; }

  private:
    std::vector<unsigned int> misc_widths_;
    unsigned int num_cores_;

    /// inputs to BD that aren't a register or memory programming word
    /// keyed by HornLeafId
    std::vector<InputInfo> input_;
    /// outputs from BD that aren't a memory dump word
    /// keyed by FunnelLeafId
    std::vector<OutputInfo> output_;
    
    /// horn description
    /// keyed by HornLeafId
    std::vector<LeafInfo> horn_;
    /// funnel description
    /// keyed by FunnelLeafId
    std::vector<LeafInfo> funnel_;

    /// memory descriptions (data field packing + misc info)
    /// keyed by MemId
    std::vector<MemInfo> mem_;

    /// memory programming words
    /// keyed by MemId
    std::vector<WordStructure> mem_prog_words_;

    /// register descriptions (data field packing + misc info)
    /// keyed by RegId
    std::vector<RegInfo> reg_;

    // funnel/horn route tables are direct-mapped
    std::vector<FHRoute> horn_routes_;
    std::vector<FHRoute> funnel_routes_;

};

} // bdpars
} // bddriver
} // pystorm

#endif
