#ifndef DRIVER_H
#define DRIVER_H   

#include <unordered_map>
#include <vector>

#include "common/BDPars.h"
#include "common/BDState.h"
#include "common/DriverTypes.h"
#include "common/DriverPars.h"
#include "encoder/Encoder.h"
#include "decoder/Decoder.h"
#include "common/MutexBuffer.h"
#include "comm/Comm.h"
#include "comm/CommSoft.h"

/* 
 * TODO LIST: funnel/horn leaves that still need their calls finished
 *
 * horn_["RI"]                   (SendTags needs impl)
 * horn_["INIT_FIFO_DCT"]        (InitFIFO needs impl)
 * horn_["INIT_FIFO_HT"]         (InitFIFO needs impl)
 * horn_["NeuronConfig"]         (SetTileSRAMMemory needs impl)
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
 * funnel_["DUMP_PRE_FIFO"]      (SetPreFIFODumpState/GetPreFIFODump needs impl)
 * funnel_["DUMP_POST_FIFO[0]"]  (SetPostFIFO0DumpState/GetPostFIFO0Dump needs impl)
 * funnel_["DUMP_POST_FIFO[1]"]  (SetPostFIFO1DumpState/GetPostFIFO1Dump needs impl)
 * funnel_["OVFLW[0]"]           (GetFIFOOverflows needs impl)
 * funnel_["OVFLW[1]"]           (GetFIFOOverflows needs impl)
 *
 * XXX need to figure out FPGA-generated/binned spike/tag stream supported functionality/calls
 */

namespace pystorm {
namespace bddriver {

/**
 * \class Driver is a singleton; There should only be one instance of this.
 *
 */

class Driver
{

  public:
    /// Return a global instance of bddriver
    static Driver * GetInstance();

    void testcall(const std::string& msg);

    /// starts child workers, e.g. encoder and decoder
    void Start(); 
    /// stops the child workers
    void Stop();
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
    void SetDACValue(unsigned int core_id, bdpars::DACSignalId signal_id,  unsigned int value);
    /// Make DAC-to-ADC connection for calibration for a particular DAC
    void SetDACtoADCConnectionState(unsigned int core_id, bdpars::DACSignalId dac_signal_id, bool en);

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
        const std::vector<MMData> & data, ///< data to program
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

    /// Dump copy of traffic pre-FIFO
    void SetPreFIFODumpState(unsigned int core_id, bool dump_en);
    /// Dump copy of traffic post-FIFO, tag msbs = 0
    void SetPostFIFO0DumpState(unsigned int core_id, bool dump_en);
    /// Dump copy of traffic post-FIFO, tag msbs = 1
    void SetPostFIFO1DumpState(unsigned int core_id, bool dump_en);
    
    /// Get pre-FIFO tags recorded during dump
    std::vector<Tag> GetPreFIFODump(unsigned int core_id, unsigned int n_tags);
    /// Get post-FIFO tags msbs = 0 recorded during dump
    std::vector<Tag> GetPostFIFO0Dump(unsigned int core_id, unsigned int n_tags);
    /// Get post-FIFO tags msbs = 1 recorded during dump
    std::vector<Tag> GetPostFIFO1Dump(unsigned int core_id, unsigned int n_tags);

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

    /// Get warning count
    std::pair<unsigned int, unsigned int> GetFIFOOverflowCounts();

    ////////////////////////////////
    // BDState queries
    
    // XXX note that these queries are NOT subject to timing assumptions!
    // this is the SOFTWARE state of the board
    // for any purpose where a timing assumption has some functional importance, 
    // there is a separate call, e.g. for traffic registers
    
    /// Get register contents by name. 
    /// XXX this is more low-level than most calls (no other public call requires RegId).
    /// Could break into multiple calls
    inline const std::pair<const std::vector<unsigned int> *, bool> GetRegState(unsigned int core_id, bdpars::RegId reg_id) const { return bd_state_[core_id].GetReg(reg_id); }

    /// Get software state of PAT memory contents: this DOES NOT dump the memory.
    inline const std::vector<PATData>  * GetPATState(unsigned int core_id) const { return bd_state_[core_id].GetPAT(); }
    /// Get software state of TAT0 memory contents: this DOES NOT dump the memory.
    inline const std::vector<TATData> * GetTAT0State(unsigned int core_id) const { return bd_state_[core_id].GetTAT0(); }
    /// Get software state of TAT1 memory contents: this DOES NOT dump the memory.
    inline const std::vector<TATData> * GetTAT1State(unsigned int core_id) const { return bd_state_[core_id].GetTAT1(); }
    /// Get software state of AM memory contents: this DOES NOT dump the memory.
    inline const std::vector<AMData>    * GetAMState(unsigned int core_id) const { return bd_state_[core_id].GetAM(); }
    /// Get software state of MM memory contents: this DOES NOT dump the memory.
    inline const std::vector<MMData>    * GetMMState(unsigned int core_id) const { return bd_state_[core_id].GetMM(); }


  private:
    Driver();
    ~Driver();

    ////////////////////////////////
    // data members

    /// parameters describing parameters of the software (e.g. buffer depths)
    driverpars::DriverPars * driver_pars_; 
    /// parameters describing BD hardware
    bdpars::BDPars * bd_pars_; 
    /// best-of-driver's-knowledge state of bd hardware
    std::vector<BDState> bd_state_;

    // downstream buffers

    /// thread-safe, MPMC buffer between breadth of downstream driver API and the encoder
    MutexBuffer<EncInput> * enc_buf_in_;
    /// thread-safe, MPMC buffer between the encoder and comm
    MutexBuffer<EncOutput> * enc_buf_out_;

    // upstream buffers

    /// thread-safe, MPMC buffer between comm and decoder
    MutexBuffer<DecInput> * dec_buf_in_;

    /// vector of thread-safe, MPMC buffers between decoder and breadth of upstream driver API
    std::vector<MutexBuffer<DecOutput> *> dec_bufs_out_;

    /// encodes traffic to BD
    Encoder * enc_; 
    /// decodes traffic from BD
    Decoder * dec_; 

    /// comm module talks to libUSB or to file
    comm::Comm * comm_;

    ////////////////////////////////
    // traffic helpers
    const std::vector<bdpars::RegId> kTrafficRegs = {
        bdpars::NeuronDumpToggle, 
        bdpars::TOGGLE_PRE_FIFO, 
        bdpars::TOGGLE_POST_FIFO0, 
        bdpars::TOGGLE_POST_FIFO1
    };

    std::unordered_map<unsigned int, std::vector<bool> > last_traffic_state_;

    /// Stops traffic for a core and saves the previous state in last_traffic_state_
    void PauseTraffic(unsigned int core_id);
    void ResumeTraffic(unsigned int core_id);

    ////////////////////////////////
    // helpers
    
    uint64_t ValueForSpecialFieldId(bdpars::WordFieldId field_id) const;
    uint64_t PackWord(const bdpars::WordStructure & word_struct, const FieldValues & field_values) const;
    std::vector<uint64_t> PackWords(const bdpars::WordStructure & word_struct, const FieldVValues & field_values) const;
    FieldValues UnpackWord(const bdpars::WordStructure & word_struct, uint64_t word) const;
    FieldVValues UnpackWords(const bdpars::WordStructure & word_struct, std::vector<uint64_t> words) const;

    void SendToHorn(unsigned int core_id, bdpars::HornLeafId leaf_id, std::vector<uint64_t> payload);
    void SendToHorn(
        const std::vector<unsigned int> & core_id, 
        const std::vector<bdpars::HornLeafId> & leaf_id,
        const std::vector<uint64_t> & payload
    );


    ////////////////////////////////
    // low-level programming calls, breadth of high-level downstream API goes through these
    
    std::vector<uint64_t> PackRWProgWords(
        const bdpars::WordStructure & word_struct, 
        const std::vector<uint64_t> & payload, 
        unsigned int start_addr
    ) const;
    std::vector<uint64_t> PackRWDumpWords(
        const bdpars::WordStructure & word_struct, 
        unsigned int start_addr,
        unsigned int end_addr
    ) const;

    std::vector<uint64_t> PackRIWIProgWords(
        const bdpars::WordStructure & addr_word_struct, 
        const bdpars::WordStructure & write_word_struct, 
        const std::vector<uint64_t> & payload, 
        unsigned int start_addr
    ) const;
    std::vector<uint64_t> PackRIWIDumpWords(
        const bdpars::WordStructure & addr_word_struct, 
        const bdpars::WordStructure & read_word_struct, 
        unsigned int start_addr,
        unsigned int end_addr
    ) const;

    std::vector<uint64_t> PackRMWProgWords(
        const bdpars::WordStructure & addr_word_struct, 
        const bdpars::WordStructure & write_word_struct, 
        const bdpars::WordStructure & incr_word_struct,
        const std::vector<uint64_t> & payload, 
        unsigned int start_addr
    ) const; 
    // note: RMWProgWord can function as RMWDumpWord

    std::vector<uint64_t> PackAMMMWord(bdpars::MemId AM_or_MM, const std::vector<uint64_t> & payload) const;

    
    void SetRegister(unsigned int core_id, bdpars::RegId reg_id, const FieldValues & field_vals);
    void SetToggle(unsigned int core_id, bdpars::RegId toggle_id, bool traffic_enable, bool dump_enable);
    bool SetToggleTraffic(unsigned int core_id, bdpars::RegId reg_id, bool en);
    bool SetToggleDump(unsigned int core_id, bdpars::RegId reg_id, bool en);

    void SetRIWIMemory(unsigned int core_id, bdpars::MemId mem_id, unsigned int start_addr, const std::vector<unsigned int> vals);
    void SetRMWMemory(unsigned int core_id, bdpars::MemId mem_id, unsigned int start_addr, const std::vector<unsigned int> vals);
    void SetRWMemory(unsigned int core_id, bdpars::MemId mem_id, unsigned int start_addr, const std::vector<unsigned int> vals);
    void SetMemoryDelay(unsigned int core_id, bdpars::MemId mem_id, unsigned int value);

    void SetTileSRAMMemory(unsigned int core_id, const std::vector<unsigned int> vals);

    void InitFIFO(unsigned int core_id);

};

} // bddriver namespace
} // pystorm namespace

#endif
