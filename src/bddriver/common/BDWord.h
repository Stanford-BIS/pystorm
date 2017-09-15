

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

/// Receiver packet structure for neuron config memory
DEFVALWORD8(NeuronConfig,
            ROW_HI,     2, 0, // [1:0]   = Row 2:1
            COL_HI,     1, 0, // [2]     = Col 2
            ROW_LO,     1, 0, // [3]     = Row 0
            COL_LO,     2, 0, // [5:4]   = Col 1:0
            BIT_VAL,    1, 0, // [6]     = Bit value
            BIT_SEL,    1, 0, // [7]     = Bitcell select
            FIXED_2,    2, 2, // [9:8]   = {1, 0}
            TILE_ADDR,  8, 0) // [17:10] = One of 256 tiles

