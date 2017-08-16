#ifndef BDPARS_H
#define BDPARS_H

#include <iostream>
#include <map>
#include <string>
#include <array>
#include <typeinfo>
#include <typeindex>

#include "binary_util.h"

using std::cout;
using std::endl;

// Syntactic sugar for defining word structures.
// Don't be confused that these are structs: they're 
// never instantiated--just passed as template parameters.
// Think of these are more like namespaces (which can't be passed to templates)
//
// XXX interface is pretty well-separated from implemention:
// this is all behind-the-scenes, it could be possible to come up with
// an implementation that doesn't involve macros

// the only difference between DEFWORDN and DEFVALWORDN is that with VAL words,
// you are allowed to specify hardcoded field values (for FIXED_M fields).

#define DEFVALWORD1(NAME, F0, W0, V0) \
struct NAME { \
  static constexpr unsigned int n_fields {1}; \
  static constexpr unsigned int field_widths[1] {W0}; \
  static constexpr unsigned int width {W0}; \
  static constexpr uint64_t field_hard_values[1] {V0}; \
  enum Field {F0}; \
}; 

#define DEFWORD1(NAME, F0, W0) \
struct NAME { \
  static constexpr unsigned int n_fields {1}; \
  static constexpr unsigned int field_widths[1] {W0}; \
  static constexpr unsigned int width {W0}; \
  static constexpr uint64_t field_hard_values[1] {0}; \
  enum Field {F0}; \
};

#define DEFVALWORD2(NAME, F0, W0, V0, F1, W1, V1) \
struct NAME { \
  static constexpr unsigned int n_fields {2}; \
  static constexpr unsigned int field_widths[2] {W0, W1}; \
  static constexpr unsigned int width {W0 + W1}; \
  static constexpr uint64_t field_hard_values[2] {V0, V1}; \
  enum Field {F0, F1}; \
};

#define DEFWORD2(NAME, F0, W0, F1, W1) \
struct NAME { \
  static constexpr unsigned int n_fields {2}; \
  static constexpr unsigned int field_widths[2] {W0, W1}; \
  static constexpr unsigned int width {W0 + W1}; \
  static constexpr uint64_t field_hard_values[2] {0, 0}; \
  enum Field {F0, F1}; \
};

#define DEFVALWORD3(NAME, F0, W0, V0, F1, W1, V1, F2, W2, V2) \
struct NAME { \
  static constexpr unsigned int n_fields {3}; \
  static constexpr unsigned int field_widths[3] {W0, W1, W2}; \
  static constexpr unsigned int width {W0 + W1 + W2}; \
  static constexpr uint64_t field_hard_values[3] {V0, V1, V2}; \
  enum Field {F0, F1, F2}; \
};

#define DEFWORD3(NAME, F0, W0, F1, W1, F2, W2) \
struct NAME { \
  static constexpr unsigned int n_fields {3}; \
  static constexpr unsigned int field_widths[3] {W0, W1, W2}; \
  static constexpr unsigned int width {W0 + W1 + W2}; \
  static constexpr uint64_t field_hard_values[3] {0, 0, 0}; \
  enum Field {F0, F1, F2}; \
};

#define DEFVALWORD4(NAME, F0, W0, V0, F1, W1, V1, F2, W2, V2, F3, W3, V3) \
struct NAME { \
  static constexpr unsigned int n_fields {4}; \
  static constexpr unsigned int field_widths[4] {W0, W1, W2, W3}; \
  static constexpr unsigned int width {W0 + W1 + W2 + W3}; \
  static constexpr uint64_t field_hard_values[4] {V0, V1, V2, V3}; \
  enum Field {F0, F1, F2, F3}; \
};

#define DEFWORD4(NAME, F0, W0, F1, W1, F2, W2, F3, W3) \
struct NAME { \
  static constexpr unsigned int n_fields = {4}; \
  static constexpr unsigned int field_widths[4] {W0, W1, W2, W3}; \
  static constexpr unsigned int width {W0 + W1 + W2 + W3}; \
  static constexpr uint64_t field_hard_values[4] {0, 0, 0, 0}; \
  enum Field {F0, F1, F2, F3}; \
};

#define DEFVALWORD5(NAME, F0, W0, V0, F1, W1, V1, F2, W2, V2, F3, W3, V3, F4, W4, V4) \
struct NAME { \
  static constexpr unsigned int n_fields {5}; \
  static constexpr unsigned int field_widths[5] {W0, W1, W2, W3, W4}; \
  static constexpr unsigned int width {W0 + W1 + W2 + W3 + W4}; \
  static constexpr uint64_t field_hard_values[5] {V0, V1, V2, V3, V4}; \
  enum Field {F0, F1, F2, F3, F4}; \
};

#define DEFVALWORD7(NAME, F0, W0, V0, F1, W1, V1, F2, W2, V2, F3, W3, V3, F4, W4, V4, F5, W5, V5, F6, W6, V6) \
struct NAME { \
  static constexpr unsigned int n_fields {7}; \
  static constexpr unsigned int field_widths[7] {W0, W1, W2, W3, W4, W5, W6}; \
  static constexpr unsigned int width {W0 + W1 + W2 + W3 + W4 + W5 + W6}; \
  static constexpr uint64_t field_hard_values[7] {V0, V1, V2, V3, V4, V5, V6}; \
  enum Field {F0, F1, F2, F3, F4, F5, F6}; \
}; \

// XXX Something else you could try: cramming all the values into the enums
// all we really need is the width and the shift for each field, and the hardcoded
// value across all fields, If you have that, you can do the necessary operations 
// (including computing the mask) without referring to the width of other fields. 
// e.g. 

/*
// lsb -> msb
//    32   |  32
// [ width | shift ]
#define DEFWORD2(NAME, F0, W0, F1, W1) \
enum class NAME { \
  F0 =  CAST32(W0), \
  F1 = (CAST32(W0) << SS) | CAST32(W1), \
  HARD_VALUE = 0};
*/

/*
#define DEFVALWORD1(NAME, F0, W0, V0) \
enum class NAME { \
  width0 = W0, \
  shift0 = 0, \
  val0 = V0, \
  F0 = (width0 << 32) | shift0, \
  HARD_VALUE = val0};

#define DEFVALWORD2(NAME, F0, W0, V0, F1, W1, V1) \
enum class NAME { \
  width0 = W0, \
  width1 = W1, \
  shift0 = 0, \
  shift1 = shift0 + width0, \
  val0 = V0, \
  val1 = V1 << shift1, \
  F0 = (width0 << WSHIFT) | shift0, \
  F1 = (width1 << WSHIFT) | shift1, \
  HARD_VALUE = val0 | val1};

#define DEFVALWORD3(NAME, F0, W0, V0, F1, W1, V1, F2, W2, V2) \
enum class NAME { \
  width0 = W0, \
  width1 = W1, \
  width2 = W2, \
  shift0 = 0, \
  shift1 = shift0 + width0, \
  shift2 = shift1 + width1, \
  val0 = V0, \
  val1 = V1 << shift1, \
  val2 = V2 << shift2, \
  F0 = (width0 << WSHIFT) | shift0, \
  F1 = (width1 << WSHIFT) | shift1, \
  F2 = (width2 << WSHIFT) | shift2, \
  HARD_VALUE = val0 | val1 | val2};

#define DEFVALWORD4(NAME, F0, W0, V0, F1, W1, V1, F2, W2, V2, F3, W3, V3) \
enum class NAME { \
  width0 = W0, \
  width1 = W1, \
  width2 = W2, \
  width3 = W3, \
  shift0 = 0, \
  shift1 = shift0 + width0, \
  shift2 = shift1 + width1, \
  shift3 = shift2 + width2, \
  val0 = V0, \
  val1 = V1 << shift1, \
  val2 = V2 << shift2, \
  val3 = V3 << shift3, \
  F0 = (width0 << WSHIFT) | shift0, \
  F1 = (width1 << WSHIFT) | shift1, \
  F2 = (width2 << WSHIFT) | shift2, \
  F3 = (width3 << WSHIFT) | shift3, \
  HARD_VALUE = val0 | val1 | val2 | val3};

#define DEFVALWORD5(NAME, F0, W0, V0, F1, W1, V1, F2, W2, V2, F3, W3, V3, F4, W4, V4) \
enum class NAME { \
  width0 = W0, \
  width1 = W1, \
  width2 = W2, \
  width3 = W3, \
  width4 = W4, \
  shift0 = 0, \
  shift1 = shift0 + width0, \
  shift2 = shift1 + width1, \
  shift3 = shift2 + width2, \
  shift4 = shift3 + width3, \
  val0 = V0, \
  val1 = V1 << shift1, \
  val2 = V2 << shift2, \
  val3 = V3 << shift3, \
  val4 = V4 << shift4, \
  F0 = (width0 << WSHIFT) | shift0, \
  F1 = (width1 << WSHIFT) | shift1, \
  F2 = (width2 << WSHIFT) | shift2, \
  F3 = (width3 << WSHIFT) | shift3, \
  F4 = (width4 << WSHIFT) | shift4, \
  HARD_VALUE = val0 | val1 | val2 | val3 | val4};

#define DEFVALWORD7(NAME, F0, W0, V0, F1, W1, V1, F2, W2, V2, F3, W3, V3, F4, W4, V4, F5, W5, V5, F6, W6, V6) \
enum class NAME { \
  width0 = W0, \
  width1 = W1, \
  width2 = W2, \
  width3 = W3, \
  width4 = W4, \
  width5 = W5, \
  width6 = W6, \
  shift0 = 0, \
  shift1 = shift0 + width0, \
  shift2 = shift1 + width1, \
  shift3 = shift2 + width2, \
  shift4 = shift3 + width3, \
  shift5 = shift4 + width4, \
  shift6 = shift5 + width5, \
  val0 = V0, \
  val1 = V1 << shift1, \
  val2 = V2 << shift2, \
  val3 = V3 << shift3, \
  val4 = V4 << shift4, \
  val5 = V5 << shift5, \
  val6 = V6 << shift6, \
  F0 = (width0 << WSHIFT) | shift0, \
  F1 = (width1 << WSHIFT) | shift1, \
  F2 = (width2 << WSHIFT) | shift2, \
  F3 = (width3 << WSHIFT) | shift3, \
  F4 = (width4 << WSHIFT) | shift4, \
  F5 = (width5 << WSHIFT) | shift5, \
  F6 = (width6 << WSHIFT) | shift6, \
  HARD_VALUE = val0 | val1 | val2 | val3 | val4 | val5 | val6};

#define DEFWORD1(NAME, F0, W0, V0) DEVALWORD1(F0, W0, 0)

#define DEFWORD2(NAME, F0, W0, V0, F1, W1, V1) DEFVALWORD2(NAME, F0, W0, 0, F1, W1, 0)

#define DEFWORD3(NAME, F0, W0, V0, F1, W1, V1, F2, W2, V2) DEFVALWORD3(NAME, F0, W0, 0, F1, W1, 0, F2, W2, 0)

#define DEFWORD4(NAME, F0, W0, V0, F1, W1, V1, F2, W2, V2, F3, W3, V3) DEFVALWORD4(NAME, F0, W0, 0, F1, W1, 0, F2, W2, 0, F3, W3, 0)
*/

namespace pystorm {
namespace bddriver {

// define all the word formats using the above macros

//DEFVALWORD1(foo, foo, 1)
//DEFVALWORD2(foobar, foo, 1, bar, 2)

// register words

DEFWORD2(ToggleWord, 
    TRAFFIC_ENABLE, 1, DUMP_ENABLE, 1)

DEFWORD2(DACWord,    
    DAC_TO_ADC_CONN, 1, DAC_VALUE, 10)

DEFWORD3(ADCWord,    
    ADC_SMALL_LARGE_CURRENT_0, 1, ADC_SMALL_LARGE_CURRENT_1, 1, ADC_OUTPUT_ENABLE, 1)

DEFWORD2(DelayWord,  
    READ_DELAY, 4, WRITE_DELAY, 4)

// memory words
DEFWORD4(AMWord, 
    ACCUMULATOR_VALUE, 15, THRESHOLD, 3, STOP, 1, NEXT_ADDRESS, 19)

DEFWORD1(MMWord, 
    WEIGHT, 8)

DEFWORD3(PATWord, 
    AM_ADDRESS, 10, MM_ADDRESS_LO, 8, MM_ADDRESS_HI, 2)

DEFVALWORD4(TATAccWord, 
    STOP,        1, 0, 
    FIXED_0,     2, 0,
    AM_ADDRESS, 10, 0,
    MM_ADDRESS, 16, 0)

DEFVALWORD7(TATSpikeWord, 
    STOP,               1, 0, 
    FIXED_1,            2, 1,
    SYNAPSE_ADDRESS_0, 11, 0,
    SYNAPSE_SIGN_0,     1, 0,
    SYNAPSE_ADDRESS_1, 11, 0,
    SYNAPSE_SIGN_1,     1, 0,
    UNUSED,             2, 0)

DEFVALWORD5(TATTagWord, 
    STOP,          1, 0,
    FIXED_2,       2, 2, 
    TAG,          11, 0,
    GLOBAL_ROUTE, 12, 0,
    UNUSED,        3, 0)

// memory programming words
DEFVALWORD3(PATWrite,
    ADDRESS, 6, 0,
    FIXED_0, 1, 0,
    DATA,   20, 0)

DEFVALWORD3(PATRead,
    ADDRESS, 6, 0,
    FIXED_1, 1, 1,
    UNUSED, 20, 0)

DEFVALWORD3(TATSetAddress,
    FIXED_0, 2, 0,
    ADDRESS, 10, 0,
    UNUSED,  19, 0)

DEFVALWORD2(TATWriteIncrement,   
    FIXED_1, 2, 1,
    DATA,   29, 0)

DEFVALWORD2(TATReadIncrement,    
    FIXED_2, 2, 2,
    UNUSED, 29, 0)

DEFVALWORD3(MMSetAddress,        
    FIXED_0,  2, 0,
    ADDRESS, 16, 0,
    UNUSED,  22, 0)

DEFVALWORD3(MMWriteIncrement,    
    FIXED_1, 2, 1,
    DATA,    8, 0,
    UNUSED, 30, 0)

DEFVALWORD2(MMReadIncrement,     
    FIXED_2, 2, 2,
    UNUSED, 38, 0)

DEFVALWORD3(AMSetAddress,        
    FIXED_0,  2, 0,
    ADDRESS, 10, 0,
    UNUSED,  28, 0)

DEFVALWORD2(AMReadWrite,         
    FIXED_1, 2, 1,
    DATA,   38, 0)

DEFVALWORD2(AMIncrement,          
    FIXED_2, 2, 2,
    UNUSED, 38, 0)

DEFVALWORD3(AMEncapsulation,      
    FIXED_0,   1, 0,
    PAYLOAD,  40, 0,
    AMMM_STOP, 1, 0)

DEFVALWORD3(MMEncapsulation,      
    FIXED_1,   1, 1,
    PAYLOAD,  40, 0,
    AMMM_STOP, 1, 0)

// input words
DEFWORD2(InputTag,
    COUNT, 9, TAG, 11)

DEFWORD1(FIFOInputTag,
    TAG, 11)

DEFWORD2(InputSpike,
    SYNAPSE_SIGN, 1, SYNAPSE_ADDRESS, 10)

// output words
DEFWORD2(PreFIFOTag,
    COUNT, 9, TAG, 11)

DEFWORD2(PostFIFOTag,
    COUNT, 9, TAG, 10)

DEFWORD1(OutputSpike,
    NEURON_ADDRESS, 12)

DEFVALWORD1(OverflowTag,
    FIXED_1, 1, 1)

DEFWORD3(AccOutputTag,
    COUNT, 9, TAG, 11, GLOBAL_ROUTE, 8)

DEFWORD3(TATOutputTag,
    COUNT, 9, TAG, 11, GLOBAL_ROUTE, 12)

/// Basically just a uint64_t with some packing and unpacking methods.
/// Being a class doesn't add a lot to this: the member functions could
/// just as well act on plain uint64_ts. 
class BDWord {

 private:
  uint64_t val_;

 public:

  BDWord() { val_ = 0; }
  /// Initialize from uint64_t 
  BDWord(uint64_t val) {
    val_ = static_cast<uint64_t>(val);
  };

  /// static member function to create a new BDWord from field values, 
  /// Initialize BDWord val_ with a list of Field enum-value pairs.
  /// Typically used when forming words to send downstream.
  /// It's hard to write the equivalent ctor because templated ctors can only
  /// work through template deduction (which doesn't seem to work here)
  template <class T>
  inline static BDWord Create(std::initializer_list<std::pair<typename T::Field, uint64_t> > fields) { 
    BDWord new_word(0);

    // compute shifts (should be evalualable at compile-time!)
    unsigned int shifts[T::n_fields];
    unsigned int curr_shift = 0;
    for (unsigned int i = 0; i < T::n_fields; i++) {
      shifts[i] = curr_shift;
      curr_shift += T::field_widths[i];
    }

    for (auto& field_and_val : fields) {

      typename T::Field field_id = field_and_val.first;
      uint64_t field_val = static_cast<uint64_t>(field_and_val.second);

      unsigned int field_idx = static_cast<unsigned int>(field_id);

      new_word.val_ |= field_val << shifts[field_idx];
    }

    for (unsigned int i = 0; i < T::n_fields; i++) {

      if (T::field_hard_values[i] != 0) {
        new_word.val_ |= T::field_hard_values[i] << shifts[i];
      }
    }
    return new_word;
  }

  /// Get a sub-field of BDWord val_ at a given Field.
  /// Typically used when unpacking words as they're sent upstream.
  template <class T, class UINT>
  inline UINT At(typename T::Field field_id) const { 

    unsigned int field_idx = static_cast<unsigned int>(field_id);

    // compute mask (should be evalualable at compile-time!)
    unsigned int shift = 0;
    for (unsigned int i = 0; i < field_idx; i++) {
      shift += T::field_widths[i];
    }

    uint64_t max_field_val = (static_cast<uint64_t>(1) << T::field_widths[field_idx]) - 1;

    uint64_t mask = max_field_val << shift;

    return (val_ & mask) >> shift;
  }

  /// Syntactic sugar for At(), UINT defaults to uint64_t
  template <class T>
  inline uint64_t At(typename T::Field field_id) const { return At<T, uint64_t>(field_id); }

  /// Get the underlying value. 
  /// Useful when packing this word into another BDWord.
  template <class UINT>
  inline UINT Packed() const { return static_cast<UINT>(val_); }

  /// Syntactic sugar for Packed(), UINT defaults to uint64_t
  inline uint64_t Packed() const { return Packed<uint64_t>(); }

  /// (in)equality checks
  friend bool operator==(const BDWord& lhs, const BDWord& rhs) {
    return lhs.val_ == rhs.val_;
  }
  friend bool operator!=(const BDWord& lhs, const BDWord& rhs) {
    return !(lhs == rhs);
  }

};


namespace bdpars {

///////////////////////////////////////////////////////////
// non word-related stuff

/// Identifier for a broad class of BD components
enum ComponentTypeId {
  REG,
  MEM,
  INPUT,
  OUTPUT,

  ComponentTypeIdCount
};

/// Identifier for particular BD register
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

  RegIdCount
};

/// Identifier for particular BD memory
enum MemId {
  AM,
  MM,
  TAT0,
  TAT1,
  PAT,
  FIFO_DCT,
  FIFO_PG,

  MemIdCount
};

/// Identifier for particular BD input stream
enum InputId {
  INPUT_TAGS,
  DCT_FIFO_INPUT_TAGS,
  HT_FIFO_RESET,
  TILE_SRAM_INPUTS,
  INPUT_SPIKES,

  InputIdCount
};

/// Identifier for particular BD output stream
enum OutputId {
  PRE_FIFO_TAGS,
  POST_FIFO_TAGS0,
  POST_FIFO_TAGS1,
  OUTPUT_SPIKES,
  OVERFLOW_TAGS0,
  OVERFLOW_TAGS1,
  ACC_OUTPUT_TAGS,
  TAT_OUTPUT_TAGS,

  OutputIdCount
};

/// Identifier for particular DAC signal name
// XXX TODO fill me in, meant to support DACSignalIdToDACRegisterId
enum DACSignalId {
  PLACEHOLDER_SIGNAL_NAME,

  DACSignalIdCount
};

/// Identifier for particular BD horn leaf
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

  HornLeafIdCount
};

/// Identifier for particular BD funnel leaf
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

  FunnelLeafIdCount
};

/// Identifier for miscellaneous BD hardware width parameters
enum MiscWidthId {
  BD_INPUT,
  BD_OUTPUT,

  MiscWidthIdCount
};

/// Neuron configuration options

enum class SomaStatusId { DISABLED, ENABLED };

enum class SomaGainId { ONE_FOURTH, ONE_THIRD, ONE_HALF, ONE };

enum class SomaOffsetSignId { POSITIVE, NEGATIVE };

enum class SomaOffsetMultiplierId { ZERO, ONE, TWO, THREE };

enum class SynapseStatusId { ENABLED, DISABLED };

enum class DiffusorCutStatusId { CLOSE, OPEN };

enum class DiffusorCutLocationId { NORTH_LEFT, NORTH_RIGHT, WEST_TOP, WEST_BOTTOM };

/// route val, route len
typedef std::pair<uint32_t, unsigned int> FHRoute;

struct LeafInfo {
  /// Information describing one funnel/horn leaf in Braindrop
  ComponentTypeId component_type;
  unsigned int component;
  uint32_t route_val;          ///< route to leaf
  unsigned int route_len;      ///< depth in funnel/horn tree (# bits in route_val)
  unsigned int data_width;     ///< width of data at leaf
  unsigned int serialization;  ///< number of serializers/deserializers
  unsigned int chunk_width;    ///< width of data before/after any deserializers/serializers
  std::string description;     ///< brief description of leaf purpose
};

struct MemInfo {
  unsigned int size;
  HornLeafId prog_leaf;
  FunnelLeafId dump_leaf;
  RegId delay_reg;
};

struct RegInfo {
  HornLeafId leaf;
};

struct InputInfo {
  HornLeafId leaf;
};

struct OutputInfo {
  FunnelLeafId leaf;
};

/// BDPars holds all the nitty-gritty information about the BD hardware's parameters.
///
/// BDPars contains several array data members containing structs, keyed by enums.
/// The enums refer to particular hardware elements or concepts, such as the name of a memory,
/// register, or a particular type of programming word.
/// The structs contain relevant information about this hardware element or concept,
/// such as the size of the memory, or the WordStructure that describes the programming word type.
class BDPars {
 public:
  BDPars();

  /////////////////////////////////////
  // funnel/horn queries

  /// Get the route to a given horn leaf
  inline FHRoute HornRoute(HornLeafId leaf) const { return horn_routes_[leaf]; }
  /// Get the route to a given horn leaf
  inline FHRoute HornRoute(unsigned int leaf) const { return horn_routes_[leaf]; }
  /// Get the route to a given funnel leaf
  inline FHRoute FunnelRoute(FunnelLeafId leaf) const { return funnel_routes_[leaf]; }
  /// Get the route to a given funnel leaf
  inline FHRoute FunnelRoute(unsigned int leaf) const { return funnel_routes_[leaf]; }

  /// Horn leaf ids may be used to index tables, this performs that mapping
  inline unsigned int HornIdx(HornLeafId leaf) const { return static_cast<unsigned int>(leaf); }
  /// Funnel leaf ids may be used to index tables, this performs that mapping
  inline unsigned int FunnelIdx(FunnelLeafId leaf) const { return static_cast<unsigned int>(leaf); }

  // useful if you need to iterate through routing table, returns const ptr
  /// return reference to Horn routing table
  inline const std::array<FHRoute, HornLeafIdCount>* HornRoutes() const { return &horn_routes_; }
  /// return reference to Funnel routing table
  inline const std::array<FHRoute, FunnelLeafIdCount>* FunnelRoutes() const { return &funnel_routes_; }

  // look up serialization
  /// Get serialization for a given horn leaf.
  /// This many messages are concatenated at the horn leaf before being sent on.
  inline unsigned int Serialization(HornLeafId leaf) const { return horn_.at(leaf).serialization; }
  /// Get serialization for a given funnel leaf
  /// The driver should concatenate this many messages from this leaf before interpreting it.
  inline unsigned int Serialization(FunnelLeafId leaf) const { return funnel_.at(leaf).serialization; }

  // going from a component Id to the FH leaf it's associated with
  /// Map from a memory to it's programming horn leaf
  inline HornLeafId HornLeafIdFor(MemId object) const { return mem_.at(object).prog_leaf; }
  /// Map from a register to it's horn leaf
  inline HornLeafId HornLeafIdFor(RegId object) const { return reg_.at(object).leaf; }
  /// Map from an input to it's horn leaf
  inline HornLeafId HornLeafIdFor(InputId object) const { return input_.at(object).leaf; }

  /// Map from a memory to it's dump funnel leaf
  inline FunnelLeafId FunnelLeafIdFor(MemId object) const { return mem_.at(object).dump_leaf; }
  /// Map from an output to it's funnel leaf
  inline FunnelLeafId FunnelLeafIdFor(OutputId object) const { return output_.at(object).leaf; }

  inline RegId DelayRegForMem(MemId object) const { return mem_.at(object).delay_reg; }

  /// going from a FH leaf to the value of the enum associate with the type of the component it services
  inline ComponentTypeId ComponentTypeIdFor(HornLeafId leaf) const { return horn_.at(leaf).component_type; }
  /// going from a FH leaf to the value of the enum associate with the type of the component it services
  inline ComponentTypeId ComponentTypeIdFor(FunnelLeafId leaf) const { return funnel_.at(leaf).component_type; }

  /// going from a FH leaf to the value of the enum associate with the component it services
  inline unsigned int ComponentIdxFor(HornLeafId leaf) const { return horn_.at(leaf).component; }
  /// going from a FH leaf to the value of the enum associate with the component it services
  inline unsigned int ComponentIdxFor(FunnelLeafId leaf) const { return funnel_.at(leaf).component; }

  /////////////////////////////////////
  // field width queries

  /// Get data width (after deserialization) at horn leaf
  inline unsigned int Width(HornLeafId object) const { return horn_.at(object).data_width; }
  /// Get data width (after deserialization) at funnel leaf
  inline unsigned int Width(FunnelLeafId object) const { return funnel_.at(object).data_width; }
  /// Get data width of some misc hardware thing
  inline unsigned int Width(MiscWidthId object) const { return misc_widths_.at(object); }

  /////////////////////////////////////
  // Word structure queries. These all look up word structures associated with the supplied object

  // Just used for testing for now:
  // (to ensure that the random test inputs aren't too big for their field)
  template <class Word>
  unsigned int WordFieldWidth(typename Word::Field field_id) const 
    { return Word::field_widths[static_cast<unsigned int>(field_id)]; }

  /////////////////////////////////////
  // misc

  /// return the capacity of a memory
  inline unsigned int Size(const MemId object) const { return mem_.at(object).size; }

  /// Map from a DAC signal name to it's register id
  RegId DACSignalIdToDACRegisterId(DACSignalId id) const;  // XXX not implemented

  /// Get the total number of registers
  inline unsigned int NumReg() const { return reg_.size(); }
  /// Get the total number of cores
  inline unsigned int NumCores() const { return num_cores_; }

  /////////////////////////////////////
  // Value Mapping/Checking for special field ids
  // note: static functions

 private:
  unsigned int num_cores_;

  /// inputs to BD that aren't a register or memory programming word
  /// keyed by HornLeafId
  std::array<InputInfo, InputIdCount> input_;
  /// outputs from BD that aren't a memory dump word
  /// keyed by FunnelLeafId
  std::array<OutputInfo, OutputIdCount> output_;

  /// horn description
  /// keyed by HornLeafId
  std::array<LeafInfo, HornLeafIdCount> horn_;
  /// funnel description
  /// keyed by FunnelLeafId
  std::array<LeafInfo, FunnelLeafIdCount> funnel_;

  /// memory descriptions (data field packing + misc info)
  /// keyed by MemId
  std::array<MemInfo, MemIdCount> mem_;

  /// register descriptions (data field packing + misc info)
  /// keyed by RegId
  std::array<RegInfo, RegIdCount> reg_;

  /// horn route tables
  /// keyed by HornLeafId
  std::array<FHRoute, HornLeafIdCount> horn_routes_;
  /// funnel route tables
  /// keyed by FunnelLeafId
  std::array<FHRoute, FunnelLeafIdCount> funnel_routes_;

  /// miscellaneous hardware widths
  /// keyed by MiscFieldId
  std::array<unsigned int, MiscWidthIdCount> misc_widths_;
};

}  // bdpars
}  // bddriver
}  // pystorm

#endif
