#ifndef DRIVER_H
#define DRIVER_H   

#include <memory>

#include "common/BDPars.h"
#include "common/DriverPars.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include "common/MutexBuffer.h"

/* 
 * TODO LIST: funnel/horn leaves that still need their calls finished
 *
 * horn_["RI"]                   (SendTags needs impl)
 * horn_["INIT_FIFO_DCT"]        (InitFIFO needs impl)
 * horn_["INIT_FIFO_HT"]         (InitFIFO needs impl)
 * horn_["NeuronConfig"]         (XXX TileSRAM calls don't exist)
 * horn_["DAC[*]"]               (SetDACtoADCConnectionState needs impl)
 * horn_["ADC"]                  (SetADCTrafficState/SetADCScale needs impl)
 * horn_["DELAY[*]"]             (SetMemDelay needs impl)
 *
 * funnel_["RO_ACC"]             (RecvTags needs impl)
 * funnel_["RO_TAT"]             (RecvTags needs impl)
 * funnel_["DUMP_AM"]            (DumpAM needs impl, see DumpPAT)
 * funnel_["DUMP_MM"]            (DumpMM needs impl, see DumpPAT)
 * funnel_["DUMP_TAT[0]"]        (DumpTAT needs impl, see DumpPAT)
 * funnel_["DUMP_TAT[1]"]        (DumpTAT needs impl, see DumpPAT)
 * funnel_["DUMP_PRE_FIFO"]      (XXX FIFO dump calls don't exist)
 * funnel_["DUMP_POST_FIFO[0]"]  (XXX FIFO dump calls don't exist)
 * funnel_["DUMP_POST_FIFO[1]"]  (XXX FIFO dump calls don't exist)
 * funnel_["OVFLW[0]"]           (XXX warning monitoring calls don't exist)
 * funnel_["OVFLW[1]"]           (XXX warning monitoring calls don't exist)
 */

namespace pystorm {
namespace bddriver {

// typedefs: words and word streams
typedef std::unordered_map<WordFieldId, uint64_t> FieldValues;
typedef std::unordered_map<WordFieldId, std::vector<uint64_t> > FieldVValues;

/**
 * \class Driver is a singleton; There should only be one instance of this.
 *
 */

class Driver
{
  public:
    /// Return a global instance of bddriver
    static Driver& getInstance();

    void testcall(const std::string& msg);

    /// starts child workers, e.g. encoder and decoder
    void Start(); 
    /// initializes hardware state
    void InitBD(); 

    ////////////////////////////////
    // Traffic Control

    /// Control tag traffic 
    void SetTagTrafficState(unsigned int core_id, bool en);

    /// Control spike traffic from neuron array to datapath
    void SetSpikeTrafficState(unsigned int core_id, bool en);
    /// Control spike traffic from neuron array to driver
    void SetSpikeDumpState(unsigned int core_id, bool en);

    ////////////////////////////////
    // ADC/DAC Config

    /// Program DAC value
    void SetDACValue(unsigned int core_id, DACSignalId signal_id,  unsigned int value);
    /// Make DAC-to-ADC connection for calibration for a particular DAC
    void SetDACtoADCConnectionState(unsigned int core_id, DACSignalId dac_signal_id, bool en);

    /// Set large/small current scale for either ADC
    void SetADCScale(unsigned int core_id, bool adc_id, const std::string & small_or_large);
    /// Turn ADC output on
    void SetADCTrafficState(unsigned int core_id, bool en);

    ////////////////////////////////
    // Neuron Config
    // ? Gains/Bias, etc

    ////////////////////////////////
    // memory programming
    
    /// Program Pool Action Table
    void SetPAT(
        unsigned int core_id, 
        const std::vector<PATData> & data, ///< data to program
        unsigned int start_addr            ///< PAT memory address to start programming from, default 0
    );

    /// Program Tag Action Table
    void SetTAT(
        unsigned int core_id, 
        bool TAT_idx,                      ///< which TAT to program, 0 or 1
        const std::vector<TATData> & data, ///< data to program
        unsigned int start_addr            ///< PAT memory address to start programming from, default 0
    );

    /// Program Accumulator Memory
    void SetAM(
        unsigned int core_id,
        const std::vector<AMData> & data, ///< data to program
        unsigned int start_addr           ///< PAT memory address to start programming from, default 0
    );

    /// Program Main Memory (a.k.a. Weight Memory)
    void SetMM(
        unsigned int core_id,
        const std::vector<unsigned int> & data, ///< data to program
        unsigned int start_addr                 ///< PAT memory address to start programming from, default 0
    );

    /// Dump PAT contents
    std::vector<PATData> DumpPAT(unsigned int core_id);
    /// Dump TAT contents
    std::vector<TATData> DumpTAT(unsigned int core_id);
    /// Dump AM contents
    std::vector<AMData> DumpAM(unsigned int core_id);
    /// Dump MM contents
    std::vector<unsigned int> DumpMM(unsigned int core_id);

    ////////////////////////////////
    // Spike/Tag Streams

    /// Send a stream of spikes to neurons
    void SendSpikes(const std::vector<Spike> & spikes);

    /// Send a stream of tags
    void SendTags(std::vector<Tag> spikes);

    /// Receive a stream of spikes
    std::vector<Spike> RecvSpikes(unsigned int max_to_recv);

    /// Receive a stream of tags
    std::vector<Tag> RecvTags(unsigned int max_to_recv);

    // XXX need to figure out FPGA-generated/binned spike/tag stream supported functionality/calls


  protected:
    Driver();
    ~Driver();

    ////////////////////////////////
    // data members

    // parameters describing parameters of the software (e.g. buffer depths)
    DriverPars * driver_pars_; 
    // parameters describing BD hardware
    BDPars * bd_pars_; 

    // thread-safe circular buffers sourcing/sinking encoder and decoder
    MutexBuffer<EncInput> * enc_buf_in_;
    MutexBuffer<EncOutput> * enc_buf_out_;

    MutexBuffer<DecInput> * dec_buf_in_;
    std::vector<MutexBuffer<DecOutput> *> dec_bufs_out_;

    Encoder * enc_; // encodes traffic to BD
    Decoder * dec_; // decodes traffic from BD

    ////////////////////////////////
    // helpers
    
    uint64_t PackWord(const WordStructure & word_struct, const FieldValues & field_values) const;
    std::vector<uint64_t> PackWords(const WordStructure & word_struct, const FieldVValues & field_values) const;
    FieldValues UnpackWord(const WordStructure & word_struct, uint64_t word) const;
    FieldVValues UnpackWords(const WordStructure & word_struct, std::vector<uint64_t> words) const;

    void SendToHorn(unsigned int core_id, HornLeafId leaf_id, std::vector<uint64_t> payload);
    void SendToHorn(
        const std::vector<unsigned int> & core_id, 
        const std::vector<HornLeafId> & leaf_id,
        const std::vector<uint64_t> & payload
    );

    uint64_t SignedValToBit(int sign) const;

    ////////////////////////////////
    // low-level programming calls, breadth of high-level downstream API goes through these
    
    std::vector<uint64_t> PackRWProgWords(
        const WordStructure & word_struct, 
        const std::vector<uint64_t> & payload, 
        unsigned int start_addr
    ) const;
    std::vector<uint64_t> PackRWDumpWords(
        const WordStructure & word_struct, 
        unsigned int start_addr,
        unsigned int end_addr
    ) const;

    std::vector<uint64_t> PackRIWIProgWords(
        const WordStructure & addr_word_struct, 
        const WordStructure & write_word_struct, 
        const std::vector<uint64_t> & payload, 
        unsigned int start_addr
    ) const;
    std::vector<uint64_t> PackRIWIDumpWords(
        const WordStructure & addr_word_struct, 
        const WordStructure & read_word_struct, 
        unsigned int start_addr,
        unsigned int end_addr
    ) const;

    std::vector<uint64_t> PackRMWProgWords(
        const WordStructure & addr_word_struct, 
        const WordStructure & write_word_struct, 
        const WordStructure & incr_word_struct,
        const std::vector<uint64_t> & payload, 
        unsigned int start_addr
    ) const; 
    // RMWProgWord == RMWDumpWord

    std::vector<uint64_t> PackAMMMWord(MemId AM_or_MM, const std::vector<uint64_t> & payload) const;

    
    void SetRegister(unsigned int core_id, RegId reg_id, const FieldValues & field_vals);
    void SetToggle(unsigned int core_id, RegId toggle_id, bool traffic_enable, bool dump_enable);

    void SetRIWIMemory(unsigned int core_id, MemId mem_id, unsigned int start_addr, const std::vector<unsigned int> vals);
    void SetRMWMemory(unsigned int core_id, MemId mem_id, unsigned int start_addr, const std::vector<unsigned int> vals);
    void SetRWMemory(unsigned int core_id, MemId mem_id, unsigned int start_addr, const std::vector<unsigned int> vals);
    void SetMemoryDelay(unsigned int core_id, MemId mem_id, unsigned int value);

    void SetTileSRAMMemory(unsigned int core_id, const std::vector<unsigned int> vals);

    void InitFIFO(unsigned int core_id);

    FieldVValues             DataToFieldVValues(const std::vector<PATData> & data) const;
    std::vector<FieldValues> DataToFieldVValues(const std::vector<TATData> & data) const; // TAT can have mixed field types
    FieldVValues             DataToFieldVValues(const std::vector<AMData> & data) const;
    FieldVValues             DataToFieldVValues(const std::vector<unsigned int> & data) const;

};

} // bddriver namespace
} // pystorm namespace

#endif
