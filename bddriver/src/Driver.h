#ifndef DRIVER_H
#define DRIVER_H   

#include <memory>

#include "common/BDPars.h"
#include "common/DriverPars.h"
#include "common/HWLoc.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include "common/MutexBuffer.h"

namespace pystorm
{
namespace bddriver
{
/**
 * \class Driver is a singleton; There should only be one instance of this.
 *
 */

typedef std::unordered_map<std::string, uint64_t> FieldValues;

struct PATData {
  /// Contents of a single PAT memory entry
  unsigned int AM_addr;
  unsigned int MM_addr_low_bits;
  unsigned int MM_addr_high_bits;
};

struct TATData {
  /// Contents of a single TAT memory entry. 
  ///
  /// Has three field classes depending on what type of data is stored
  // (this implementation is wasteful but simple).

  unsigned int stop;
  unsigned int type;

  // type == 0 means accumulator entry
  unsigned int AM_addr;
  unsigned int MM_addr;

  // type == 1 means neuron entry
  unsigned int tap_addr;
  int          tap_sign; // -1 or +1
  
  // type == 2 means fanout entry
  unsigned int tag;
  unsigned int global_route;
};

struct AMData {
  /// Contents of a single AM memory entry.
  ///
  /// (the value field is not exposed to the programmer, 
  ///  programming the AM sets this field to 0)
  unsigned int threshold;
  unsigned int stop;
  unsigned int next_addr;
};

struct Spike {
  /// A spike going to or from a neuron
  unsigned int core_id;
  unsigned int neuron_id;
  int sign;
};

struct Tag {
  /// A tag going to or from the datapath
  unsigned int core_id;
  unsigned int tag_id;
  unsigned int count;
};

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

    /// Turn on tag traffic in datapath (also calls Start/KillSpikes)
    void StartTraffic(unsigned int core_id);
    /// Turn off tag traffic in datapath (also calls Start/KillSpikes)
    void KillTraffic(unsigned int core_id);

    /// Turn on spike outputs for all neurons
    void StartSpikes(unsigned int core_id);
    /// Turn on spike output of a particular neuron
    void StartSpikes(unsigned int core_id, unsigned int neuron_id);
    /// Turn off spike outputs for all neurons
    void KillSpikes(unsigned int core_id);
    /// Turn off spike output of a particular neuron
    void KillSpikes(unsigned int core_id, unsigned int neuron_id);

    ////////////////////////////////
    // ADC/DAC Config

    /// Program DAC value
    void SetDACValue(unsigned int core_id, const std::string & dac_signal_name, unsigned int value);

    /// Make DAC-to-ADC connection for calibration for a particular DAC
    void ConnectDACtoADC(unsigned int core_id, const std::string & dac_signal_name);
    /// Disconnect DAC-to-ADC connection for all DACs
    void DisconnectDACsfromADC(unsigned int core_id);

    /// Set large/small current scale for either ADC
    void SetADCScale(unsigned int core_id, bool adc_id, const std::string & small_or_large);
    /// Turn ADC output on
    void StartADCTraffic(unsigned int core_id);
    /// Turn ADC output off
    void KillADCTraffic(unsigned int core_id);

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

    ////////////////////////////////
    // Spike/Tag Streams

    /// Send a stream of spikes to neurons
    void SendSpikes(
        const std::vector<Spike> & spikes,        ///< addresses of neurons to send spikes to
        const std::vector<unsigned int> & delays  ///< inter-spike intervals
    );

    /// Send a stream of tags
    void SendTags(
        std::vector<Spike> spikes,        ///< tag ids to send
        std::vector<unsigned int> delays  ///< inter-tag intervals
    );

    // XXX need to figure out FPGA-generated spike stream calls

    /// Receive a stream of spikes
    std::vector<Spike> RecvSpikes(unsigned int max_to_recv);

    /// Receive a stream of tags
    std::vector<Tag> RecvTags(unsigned int max_to_recv);


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

    void SendToHorn(unsigned int core_id, const std::string & leaf_name, std::vector<uint64_t> payload);
    void SendToHorn(
      const std::vector<unsigned int> & core_id, 
      const std::vector<std::string> & leaf_name, 
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

    std::vector<uint64_t> PackRIWIProgWords(
        const WordStructure & addr_word_struct, 
        const WordStructure & write_word_struct, 
        const std::vector<uint64_t> & payload, 
        unsigned int start_addr
    ) const;

    std::vector<uint64_t> PackRMWProgWords(
        const WordStructure & addr_word_struct, 
        const WordStructure & write_word_struct, 
        const WordStructure & incr_word_struct,
        const std::vector<uint64_t> & payload, 
        unsigned int start_addr
    ) const; 

    std::vector<uint64_t> PackAMMMWord(const std::string & AM_or_MM, const std::vector<uint64_t> & payload) const;

    
    void ToggleStream(unsigned int core_id, const std::string & stream_name, bool traffic_on, bool dump_on); // wrapper around SetRegister

    void SetRegister(unsigned int core_id, const std::string & reg_name, const FieldValues & field_vals);
    void SetToggle(unsigned int core_id, const std::string & toggle_name, bool traffic_enable, bool dump_enable);

    void SetRIWIMemory(unsigned int core_id, const std::string & memory_name, unsigned int start_addr, const std::vector<unsigned int> vals);
    void SetRMWMemory(unsigned int core_id, const std::string & memory_name, unsigned int start_addr, const std::vector<unsigned int> vals);
    void SetRWMemory(unsigned int core_id, const std::string & memory_name, unsigned int start_addr, const std::vector<unsigned int> vals);
    void SetMemoryDelay(unsigned int core_id, const std::string & memory_name, unsigned int value);

    void SetTileSRAMMemory(unsigned int core_id, const std::vector<unsigned int> vals);

    void InitFIFO(unsigned int core_id);

};

} // bddriver namespace
} // pystorm namespace

#endif
