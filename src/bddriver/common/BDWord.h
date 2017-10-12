#ifndef BDWORD_H
#define BDWORD_H

#include <typeindex>
#include <utility>
#include <vector>
#include <unordered_map>
#include <cassert>

// Macros for defining word parameters
// invoked like:
// DEFHCWORD(SomeWord, 
//  FIELDS(field_name1, field_name2)
//  WIDTHS(field_width1, field_width2),
//  HCVALS(hard_coded_field_val1, hard_coded_field_val2))
//
//  or
//
// DEFWORD(SomeWord, 
//  FIELDS(field_name1, field_name2)
//  WIDTHS(field_width1, field_width2)),
//
// (HCVALS defaults to all 0s)
//
// was trying to make DEFWORD have 2 or 3 args
// https://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments

// It's hard to have multiple same-length-but-variable fields in a macro
// The following three macro definitions are a workaround
#define FIELDS(...) {__VA_ARGS__, FIELDCOUNT}
#define WIDTHS(...) {__VA_ARGS__}
#define HCVALS(...) {__VA_ARGS__}

// The main macro, gives you an enum class WordType and two functions:
// FieldWidth(WordType field_name) returns width for a field_name 
// FieldHCVal(WordType field_name) returns hardcoded value for a field_name
#define DEFHCWORD(WordType, field_names, widths, hcvals) \
enum class WordType field_names; \
constexpr unsigned int FieldWidth(WordType field_name) { \
  constexpr unsigned int w[] = widths; \
  const unsigned int field_name_idx = static_cast<unsigned int>(field_name); \
  return w[field_name_idx]; \
} \
constexpr unsigned int FieldHCVal(WordType field_name) { \
  constexpr unsigned int v[] = hcvals; \
  const unsigned int field_name_idx = static_cast<unsigned int>(field_name); \
  return v[field_name_idx]; \
} 

// two-argument variation with no HCVALS
// note that the (void)field_name does nothing, it's just to squelch the compiler warning
// about field_name being unused
// https://stackoverflow.com/questions/1486904/how-do-i-best-silence-a-warning-about-unused-variables
#define DEFWORD(WordType, field_names, widths) \
enum class WordType field_names; \
constexpr unsigned int FieldWidth(WordType field_name) { \
  constexpr unsigned int w[] = widths; \
  const unsigned int field_name_idx = static_cast<unsigned int>(field_name); \
  return w[field_name_idx]; \
} \
constexpr unsigned int FieldHCVal(WordType field_name) { \
  (void)field_name; \
  return 0; \
}

namespace pystorm {
namespace bddriver {

DEFWORD(ToggleWord, 
    FIELDS(TRAFFIC_ENABLE , DUMP_ENABLE ) ,
    WIDTHS(1              , 1           )  )

DEFWORD(DACWord,
    FIELDS(DAC_TO_ADC_CONN , DAC_VALUE ) ,
    WIDTHS(1               , 10        )   )

DEFWORD(ADCWord,
    FIELDS(ADC_SMALL_LARGE_CURRENT_0 , ADC_SMALL_LARGE_CURRENT_1 , ADC_OUTPUT_ENABLE ) ,
    WIDTHS(1                         , 1                         , 1                 )   )

DEFWORD(DelayWord,
    FIELDS(READ_DELAY , WRITE_DELAY ) ,
    WIDTHS(4          , 4           )   )

// memory words
DEFWORD(AMWord,
    FIELDS(ACCUMULATOR_VALUE , THRESHOLD , STOP , NEXT_ADDRESS ) ,
    WIDTHS(15                , 3         , 1    , 19           )   )

DEFWORD(MMWord,
    FIELDS(WEIGHT ) ,
    WIDTHS(8      )   )

DEFWORD(PATWord,
    FIELDS(AM_ADDRESS , MM_ADDRESS_LO , MM_ADDRESS_HI ) ,
    WIDTHS(10         , 8             , 2             )   )

DEFHCWORD(TATAccWord,
    FIELDS(STOP , FIXED_0 , AM_ADDRESS , MM_ADDRESS ) ,
    WIDTHS(1    , 2       , 10         , 16         ) ,
    HCVALS(0    , 0       , 0          , 0          )   )

DEFHCWORD(TATSpikeWord,
    FIELDS(STOP , FIXED_1 , SYNAPSE_ADDRESS_0 , SYNAPSE_SIGN_0 , SYNAPSE_ADDRESS_1 , SYNAPSE_SIGN_1 , UNUSED ) ,
    WIDTHS(1    , 2       , 11                , 1              , 11                , 1              , 2      ) ,
    HCVALS(0    , 1       , 0                 , 0              , 0                 , 0              , 0      )   )

DEFHCWORD(TATTagWord,
    FIELDS(STOP , FIXED_2 , TAG , GLOBAL_ROUTE , UNUSED ) ,
    WIDTHS(1    , 2       , 11  , 12           , 3      ) ,
    HCVALS(0    , 2       , 0   , 0            , 0      )   )

// memory programming words
DEFHCWORD(PATWrite,
    FIELDS(ADDRESS , FIXED_0 , DATA ) ,
    WIDTHS(6       , 1       , 20   ) ,
    HCVALS(0       , 0       , 0    )   )

DEFHCWORD(PATRead,
    FIELDS(ADDRESS , FIXED_1 , UNUSED ) ,
    WIDTHS(6       , 1       , 20     ) ,
    HCVALS(0       , 1       , 0      )   )

DEFHCWORD(TATSetAddress,
    FIELDS(FIXED_0 , ADDRESS , UNUSED ) ,
    WIDTHS(2       , 10      , 19     ) ,
    HCVALS(0       , 0       , 0      )   )

DEFHCWORD(TATWriteIncrement,
    FIELDS(FIXED_1 , DATA ) ,
    WIDTHS(2       , 29   ) ,
    HCVALS(1       , 0    )   )

DEFHCWORD(TATReadIncrement,
    FIELDS(FIXED_2 , UNUSED ) ,
    WIDTHS(2       , 29     ) ,
    HCVALS(2       , 0      )   )

DEFHCWORD(MMSetAddress,
    FIELDS(FIXED_0 , ADDRESS , UNUSED ) ,
    WIDTHS(2       , 16      , 22     ) ,
    HCVALS(0       , 0       , 0      )   )

DEFHCWORD(MMWriteIncrement,
    FIELDS(FIXED_1 , DATA , UNUSED ) ,
    WIDTHS(2       , 8    , 30     ) ,
    HCVALS(1       , 0    , 0      )   )

DEFHCWORD(MMReadIncrement,
    FIELDS(FIXED_2 , UNUSED ) ,
    WIDTHS(2       , 38     ) ,
    HCVALS(2       , 0      )   )

DEFHCWORD(AMSetAddress,
    FIELDS(FIXED_0 , ADDRESS , UNUSED ) ,
    WIDTHS(2       , 10      , 28     ) ,
    HCVALS(0       , 0       , 0      )   )

DEFHCWORD(AMReadWrite,
    FIELDS(FIXED_1 , DATA ) ,
    WIDTHS(2       , 38   ) ,
    HCVALS(1       , 0    )   )

DEFHCWORD(AMIncrement,
    FIELDS(FIXED_2 , UNUSED ) ,
    WIDTHS(2       , 38     ) ,
    HCVALS(2       , 0      )   )

DEFHCWORD(AMEncapsulation,
    FIELDS(FIXED_0 , PAYLOAD , AMMM_STOP ) ,
    WIDTHS(1       , 40      , 1         ) ,
    HCVALS(0       , 0       , 0         )   )

DEFHCWORD(MMEncapsulation,
    FIELDS(FIXED_1 , PAYLOAD , AMMM_STOP ) ,
    WIDTHS(1       , 40      , 1         ) ,
    HCVALS(1       , 0       , 0         )   )

// input words
DEFWORD(InputTag,
    FIELDS(COUNT , TAG ) ,
    WIDTHS(9     , 11  )   )

DEFWORD(FIFOInputTag,
    FIELDS(TAG ) ,
    WIDTHS(11  )   )

DEFWORD(InputSpike,
    FIELDS(SYNAPSE_SIGN , SYNAPSE_ADDRESS ) ,
    WIDTHS(1            , 10              )   )

// output words
DEFWORD(PreFIFOTag,
    FIELDS(COUNT , TAG ) ,
    WIDTHS(9     , 11  )   )

DEFWORD(PostFIFOTag,
    FIELDS(COUNT , TAG ) ,
    WIDTHS(9     , 10  )   )

DEFWORD(OutputSpike,
    FIELDS(NEURON_ADDRESS ) ,
    WIDTHS(12             )   )

DEFHCWORD(OverflowTag,
    FIELDS(FIXED_1 ) ,
    WIDTHS(1       ) ,
    HCVALS(1       )   )

DEFWORD(AccOutputTag,
    FIELDS(COUNT , TAG , GLOBAL_ROUTE ) ,
    WIDTHS(9     , 11  , 8            )   )

DEFWORD(TATOutputTag,
    FIELDS(COUNT , TAG , GLOBAL_ROUTE ) ,
    WIDTHS(9     , 11  , 12           )   )

// [1:0]   = Row 2:1
// [2]     = Col 2
// [3]     = Row 0
// [5:4]   = Col 1:0
// [6]     = Bit value
// [7]     = Bitcell select
// [9:8]   = {1, 0}
// [17:10] = One of 256 tiles
DEFHCWORD(NeuronConfig,
    FIELDS(ROW_HI , COL_HI , ROW_LO , COL_LO , BIT_VAL , BIT_SEL , FIXED_2 , TILE_ADDR ) ,
    WIDTHS(2      , 1      , 1      , 2      , 1       , 1       , 2       , 8         ) ,
    HCVALS(0      , 0      , 0      , 0      , 0       , 0       , 2       , 0         )   )

// FPGA words
DEFWORD(FPGAIO,
    FIELDS(PAYLOAD , EP_CODE ) ,
    WIDTHS(24      , 8       )   )

DEFWORD(FPGABYTES,
    FIELDS(B0 , B1 , B2 , B3 ) ,
    WIDTHS(8  , 8  , 8  , 8  )   )

// for convenience/code reuse, this is used for the FPGA serdes in software
DEFWORD(TWOFPGAPAYLOADS,
    FIELDS(LSB, MSB ) ,
    WIDTHS(24 ,  24 )   )

DEFWORD(THREEFPGAREGS,
    FIELDS(W0, W1, W2 ) ,
    WIDTHS(16, 16, 16 )   )

// FPGA register words
DEFWORD(FPGABDReset,
    FIELDS(PRESET, SRESET ) ,
    WIDTHS(1     , 1      )   )

DEFWORD(FPGATMUnitLen,
    FIELDS(UNIT_LEN ) ,
    WIDTHS(16       )   )

DEFWORD(FPGAResetClock,
    FIELDS(RESET_STATE) ,
    WIDTHS(1          )   )


// There's no proper BDWord type anymore (there never was, really)
// just a typedef
typedef uint64_t BDWord;

template <class T>
uint64_t PackWord(const std::initializer_list<std::pair<T, uint64_t> > & fields) { 
  
  uint64_t retval = 0;

  // compute shifts (should be evaluable at compile-time), OR in default values
  uint64_t shifts[static_cast<unsigned int>(T::FIELDCOUNT)];
  uint64_t curr_shift = 0;
  for (unsigned int i = 0; i < static_cast<unsigned int>(T::FIELDCOUNT); i++) {
    retval |= FieldHCVal(static_cast<T>(i)) << curr_shift;
    shifts[i] = curr_shift;
    curr_shift += FieldWidth(static_cast<T>(i));
  }

  // OR in user values
  for (auto& field_and_val : fields) {
    T field      = field_and_val.first;
    uint64_t val = field_and_val.second;

    // make sure the user isn't packing too large a value into the field
    assert(val <= (static_cast<uint64_t>(1) << FieldWidth(field)) - 1);
    // make sure that the user isn't trying to set a hardcoded field
    assert(FieldHCVal(field) == 0);

    retval |= val << shifts[static_cast<unsigned int>(field)];
  }

  return retval;
}

/// Non-const form of PackWord for dynamic usage
template <class T>
uint64_t PackWordVectorized(std::vector<std::pair<T, uint64_t> > & fields) { 
  
  uint64_t retval = 0;

  // compute shifts (should be evaluable at compile-time), OR in default values
  uint64_t shifts[static_cast<unsigned int>(T::FIELDCOUNT)];
  uint64_t curr_shift = 0;
  for (unsigned int i = 0; i < static_cast<unsigned int>(T::FIELDCOUNT); i++) {
    retval |= FieldHCVal(static_cast<T>(i)) << curr_shift;
    shifts[i] = curr_shift;
    curr_shift += FieldWidth(static_cast<T>(i));
  }

  // OR in user values
  for (auto& field_and_val : fields) {
    T field      = field_and_val.first;
    uint64_t val = field_and_val.second;

    // make sure the user isn't packing too large a value into the field
    assert(val <= (static_cast<uint64_t>(1) << FieldWidth(field)) - 1);
    // make sure that the user isn't trying to set a hardcoded field
    assert(FieldHCVal(field) == 0);

    retval |= val << shifts[static_cast<unsigned int>(field)];
  }

  return retval;
}

template <class T>
uint64_t GetField(uint64_t word, T field) {

  unsigned int field_idx = static_cast<unsigned int>(field);

  // compute shifts (should be evaluable at compile-time)
  unsigned int shift = 0;
  for (unsigned int i = 0; i < field_idx; i++) {
    shift += FieldWidth(static_cast<T>(i));
  }

  uint64_t max_field_val = (static_cast<uint64_t>(1) << FieldWidth(field)) - 1;

  uint64_t mask = max_field_val << shift;

  return (word & mask) >> shift;
}

} // bddriver
} // pystorm

#endif