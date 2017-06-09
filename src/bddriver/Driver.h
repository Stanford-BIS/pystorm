#ifndef DRIVER_H
#define DRIVER_H

#include <unordered_map>
#include <vector>

#include "comm/Comm.h"
#include "comm/CommSoft.h"
#include "common/BDPars.h"
#include "common/BDState.h"
#include "common/DriverPars.h"
#include "common/DriverTypes.h"
#include "common/MutexBuffer.h"
#include "decoder/Decoder.h"
#include "encoder/Encoder.h"

/*
 * TODO LIST: funnel/horn leaves that still need their calls finished
 *
 * horn_["NeuronConfig"]         (? needs impl (Ben))
 * horn_["DAC[*]"]               (SetDACtoADCConnectionState needs impl (Ben))
 * horn_["ADC"]                  (SetADCTrafficState/SetADCScale needs impl (Ben))
 *
 * XXX need to figure out FPGA-generated/binned spike/tag stream supported functionality/calls
 */

namespace pystorm {
namespace bddriver {

/**
 * \class Driver is a singleton; There should only be one instance of this.
 *
 */

/// Driver provides low-level, but not dead-stupid, control over the BD hardware.
/// Driver tries to provide a complete but not needlessly tedious interface to BD.
/// It also tries to prevent the user to do anything that would crash the chip.
///
/// Driver looks like this:
///
///                                    (user)
///
///  ---[fns]--[fns]--[fns]------------[fns]-----------------------[fns]----  API
///       |      |      |                A                           A
///       V      V      V                |                           |
///  [private fns, e.g. PackWords]  [XXXX private fns, e.g. UnpackWords XXXX]
///          |        |                  A                           A
///          V        V                  |                           |
///   [MutexBuffer:enc_buf_in_]   [M.B.:dec_buf_out_[0]]    [M.B.:dec_buf_out_[0]] ...
///              |                             A                   A
///              V                             |                   |
///      [Encoder:encoder_]          [XXXXXXXXXXXX Decoder:decoder_ XXXXXXXXXX]
///              |                                   A
///              V                                   |
///   [MutexBuffer:enc_buf_out_]          [MutexBuffer:dec_buf_in_]
///              |                                   A                       [BDPars]
///              V                                   |                       [BDState]
///         [XXXXXXXXXXXXXXXXXX Comm:comm_ XXXXXXXXXXXXXXXXX]
///                             |      A
///                             V      |
///  ----------------------------------------------------------------------- USB
///
///                            (Braindrop)
///
/// At the heart of driver are a few primary components:
///
/// - Encoder
///     Inputs: raw payloads (already serialized, if necessary) and BD horn ids to send them to
///     Outputs: inputs suitable to send to BD, packed into char stream
///     Spawns its own thread.
///
/// - Decoder
///     Inputs: char stream of outputs from BD
///     Outputs: one stream per horn leaf of raw payloads from that leaf
///     Spawns its own thread.
///
/// - Comm
///     Communicates with BD using libUSB, taking inputs from/giving outputs to
///     the Encoder/Decoder. Spawns its own thread.
///
/// - MutexBuffers
///     Provide thread-safe communication and buffering for the inputs and outputs of Encoder
///     and decoder. Note that there are many decoder output buffers, one per funnel leaf.
///
/// - BDPars
///     Holds all the nitty-gritty hardware information. The rest of the driver doesn't know
///     anything about word field orders or sizes, for example.
///
/// - BDState
///     Software model of the hardware state. Keep track of all the memory words that have
///     been programmed, registers that have been set, etc.
///     Also keeps track of timing assumptions, e.g. whether the traffic has drained after
///     turning off all of the toggles that stop it.
///
class Driver {
 public:
  /// can supply your own comm if you're doing something funky
  Driver();
  ~Driver();

  /// Return a global instance of bddriver
  // static Driver * GetInstance();

  inline const bdpars::BDPars *GetBDPars() { return bd_pars_; }
  inline const driverpars::DriverPars *GetDriverPars() { return driver_pars_; }
  inline const BDState *GetState(unsigned int core_id) { return &bd_state_[core_id]; }

  void testcall(const std::string &msg);

  /// starts child workers, e.g. encoder and decoder
  void Start();
  /// stops the child workers
  void Stop();
  /// initializes hardware state
  void InitBD();                        // XXX partially implemented
  void InitFIFO(unsigned int core_id); 

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
  void SetDACValue(unsigned int core_id, bdpars::DACSignalId signal_id, unsigned int value);  // XXX not implemented
  /// Make DAC-to-ADC connection for calibration for a particular DAC
  void SetDACtoADCConnectionState(
      unsigned int core_id, bdpars::DACSignalId dac_signal_id, bool en);  // XXX not implemented

  /// Set large/small current scale for either ADC
  void SetADCScale(unsigned int core_id, bool adc_id, const std::string &small_or_large);  // XXX not implemented
  /// Turn ADC output on
  void SetADCTrafficState(unsigned int core_id, bool en);  // XXX not implemented

  ////////////////////////////////
  // Neuron Config
  // XXX Ben? Gains/Bias, etc

  ////////////////////////////////
  // memory programming

  /// Program Pool Action Table
  inline void SetPAT(
      unsigned int core_id,
      const std::vector<PATData> &data,  ///< data to program
      unsigned int start_addr) {         ///< memory address to start programming from, default 0
    bd_state_[core_id].SetPAT(start_addr, data);
    SetMem(core_id, bdpars::PAT, DataToVFieldValues(data), start_addr);
  }

  /// Program Tag Action Table
  inline void SetTAT(
      unsigned int core_id,
      bool TAT_idx,                      ///< which TAT to program, 0 or 1
      const std::vector<TATData> &data,  ///< data to program
      unsigned int start_addr) {         ///< memory address to start programming from, default 0
    if (TAT_idx == 0) {
      bd_state_[core_id].SetTAT0(start_addr, data);
      SetMem(core_id, bdpars::TAT0, DataToVFieldValues(data), start_addr);
    } else if (TAT_idx == 1) {
      bd_state_[core_id].SetTAT1(start_addr, data);
      SetMem(core_id, bdpars::TAT1, DataToVFieldValues(data), start_addr);
    }
  }

  /// Program Accumulator Memory
  inline void SetAM(
      unsigned int core_id,
      const std::vector<AMData> &data,  ///< data to program
      unsigned int start_addr) {        ///< memory address to start programming from, default 0
    bd_state_[core_id].SetAM(start_addr, data);
    SetMem(core_id, bdpars::AM, DataToVFieldValues(data), start_addr);
  }

  /// Program Main Memory (a.k.a. Weight Memory)
  inline void SetMM(
      unsigned int core_id,
      const std::vector<MMData> &data,  ///< data to program
      unsigned int start_addr) {        ///< memory address to start programming from, default 0
    bd_state_[core_id].SetMM(start_addr, data);
    SetMem(core_id, bdpars::MM, DataToVFieldValues(data), start_addr);
  }

  /// Dump PAT contents
  inline std::vector<PATData> DumpPAT(unsigned int core_id) 
    { return VFieldValuesToPATData(DumpMem(core_id, bdpars::PAT)); }
  /// Dump TAT contents
  inline std::vector<TATData> DumpTAT(unsigned int core_id, unsigned int tat_idx) { 
      bdpars::MemId mem_id; 
      if (tat_idx == 0) mem_id = bdpars::TAT0;
      else if (tat_idx == 1) mem_id = bdpars::TAT1;
      else assert(false);
      return VFieldValuesToTATData(DumpMem(core_id, mem_id));
  }
  /// Dump AM contents
  inline std::vector<AMData> DumpAM(unsigned int core_id) 
    { return VFieldValuesToAMData(DumpMem(core_id, bdpars::AM)); }
  /// Dump MM contents
  inline std::vector<MMData> DumpMM(unsigned int core_id) 
    { return VFieldValuesToMMData(DumpMem(core_id, bdpars::MM)); }

  /// Dump copy of traffic pre-FIFO
  void SetPreFIFODumpState(unsigned int core_id, bool dump_en);
  /// Dump copy of traffic post-FIFO, tag msbs = 0
  void SetPostFIFODumpState(unsigned int core_id, bool dump_en);

  /// Get pre-FIFO tags recorded during dump
  std::vector<Tag> GetPreFIFODump(unsigned int core_id, unsigned int n_tags);
  /// Get post-FIFO tags recorded during dump
  std::vector<Tag> GetPostFIFODump(unsigned int core_id, unsigned int n_tags0, unsigned int n_tags1);

  /// Get warning count
  std::pair<unsigned int, unsigned int> GetFIFOOverflowCounts(unsigned int core_id);

  ////////////////////////////////
  // Spike/Tag Streams

  /// Send a stream of spikes to neurons
  void SendSpikes(const std::vector<SynSpike> &spikes);

  /// Receive a stream of spikes
  std::vector<NrnSpike> RecvSpikes(unsigned int max_to_recv);

  /// Send a stream of tags
  void SendTags(const std::vector<Tag> &tags);

  /// Receive a stream of tags
  std::vector<Tag> RecvTags(unsigned int max_to_recv);

  ////////////////////////////////
  // BDState queries

  // XXX note that these queries are NOT subject to timing assumptions!
  // this is the SOFTWARE state of the board
  // for any purpose where a timing assumption has some functional importance,
  // there is a separate call, e.g. for traffic registers

  /// Get register contents by name.
  /// XXX this is more low-level than most calls (no other public call requires RegId).
  /// Could break into multiple calls
  inline const std::pair<const std::vector<unsigned int> *, bool> GetRegState(
      unsigned int core_id, bdpars::RegId reg_id) const {
    return bd_state_[core_id].GetReg(reg_id);
  }

  /// Get software state of PAT memory contents: this DOES NOT dump the memory.
  inline const std::vector<PATData> *GetPATState(unsigned int core_id) const { return bd_state_[core_id].GetPAT(); }
  /// Get software state of TAT0 memory contents: this DOES NOT dump the memory.
  inline const std::vector<TATData> *GetTAT0State(unsigned int core_id) const { return bd_state_[core_id].GetTAT0(); }
  /// Get software state of TAT1 memory contents: this DOES NOT dump the memory.
  inline const std::vector<TATData> *GetTAT1State(unsigned int core_id) const { return bd_state_[core_id].GetTAT1(); }
  /// Get software state of AM memory contents: this DOES NOT dump the memory.
  inline const std::vector<AMData> *GetAMState(unsigned int core_id) const { return bd_state_[core_id].GetAM(); }
  /// Get software state of MM memory contents: this DOES NOT dump the memory.
  inline const std::vector<MMData> *GetMMState(unsigned int core_id) const { return bd_state_[core_id].GetMM(); }

 protected:
  ////////////////////////////////
  // data members

  /// parameters describing parameters of the software (e.g. buffer depths)
  driverpars::DriverPars *driver_pars_;
  /// parameters describing BD hardware
  bdpars::BDPars *bd_pars_;
  /// best-of-driver's-knowledge state of bd hardware
  std::vector<BDState> bd_state_;

  // downstream buffers

  /// thread-safe, MPMC buffer between breadth of downstream driver API and the encoder
  MutexBuffer<EncInput> *enc_buf_in_;
  /// thread-safe, MPMC buffer between the encoder and comm
  MutexBuffer<EncOutput> *enc_buf_out_;

  // upstream buffers

  /// thread-safe, MPMC buffer between comm and decoder
  MutexBuffer<DecInput> *dec_buf_in_;

  /// vector of thread-safe, MPMC buffers between decoder and breadth of upstream driver API
  std::vector<MutexBuffer<DecOutput> *> dec_bufs_out_;

  /// encodes traffic to BD
  Encoder *enc_;
  /// decodes traffic from BD
  Decoder *dec_;

  /// comm module talks to libUSB or to file
  comm::Comm *comm_;

  ////////////////////////////////
  // traffic helpers
  const std::vector<bdpars::RegId> kTrafficRegs = {
      bdpars::NEURON_DUMP_TOGGLE, bdpars::TOGGLE_PRE_FIFO, bdpars::TOGGLE_POST_FIFO0, bdpars::TOGGLE_POST_FIFO1};

  std::unordered_map<unsigned int, std::vector<bool> > last_traffic_state_;

  /// Stops traffic for a core and saves the previous state in last_traffic_state_
  void PauseTraffic(unsigned int core_id);
  void ResumeTraffic(unsigned int core_id);

  ////////////////////////////////
  // helpers

  std::pair<std::vector<uint32_t>, unsigned int> SerializeWordsToLeaf(
      const std::vector<uint64_t> &inputs, bdpars::HornLeafId) const;
  std::pair<std::vector<uint64_t>, std::vector<uint32_t> > DeserializeWordsFromLeaf(
      const std::vector<uint32_t> &inputs, bdpars::FunnelLeafId leaf_id) const;

  /// Sends a vector of payloads to a single <core_id> and <leaf_id>.
  ///
  /// Determines whether payloads must be serialized based on <leaf_id>.
  /// For payloads that don't need to be serialized, or that might need to
  /// go to multiple cores, this isn't the most effective call.
  /// SendSpikes(), for example, pushes directly to the encoder's input buffer.
  void SendToHorn(unsigned int core_id, bdpars::HornLeafId leaf_id, const std::vector<uint64_t> &payload);

  /// Pops from a <leaf_id>'s dec_bufs_out_[] <num_to_recv> payloads
  /// associated with a supplied <core_id>.
  ///
  /// If <num_to_recv> is zero, then receive whatever's currently in the buffer.
  /// Returns vector of deserialized payloads. For payloads that don't need to be deserialized,
  /// that might come from multiple cores, or that need time_epoch information,
  /// This isn't the most effective call.
  std::vector<uint64_t> RecvFromFunnel(
      unsigned int core_id, 
      bdpars::FunnelLeafId leaf_id, 
      unsigned int num_to_recv = 0);

  ////////////////////////////////
  // memory programming helpers

  std::vector<uint64_t> PackRWProgWords(
      const bdpars::WordStructure &word_struct, const std::vector<uint64_t> &payload, unsigned int start_addr) const;
  std::vector<uint64_t> PackRWDumpWords(
      const bdpars::WordStructure &word_struct, unsigned int start_addr, unsigned int end_addr) const;

  std::vector<uint64_t> PackRIWIProgWords(
      const bdpars::WordStructure &addr_word_struct,
      const bdpars::WordStructure &write_word_struct,
      const std::vector<uint64_t> &payload,
      unsigned int start_addr) const;
  std::vector<uint64_t> PackRIWIDumpWords(
      const bdpars::WordStructure &addr_word_struct,
      const bdpars::WordStructure &read_word_struct,
      unsigned int start_addr,
      unsigned int end_addr) const;

  std::vector<uint64_t> PackRMWProgWords(
      const bdpars::WordStructure &addr_word_struct,
      const bdpars::WordStructure &write_word_struct,
      const bdpars::WordStructure &incr_word_struct,
      const std::vector<uint64_t> &payload,
      unsigned int start_addr) const;
  // note: RMWProgWord can function as RMWDumpWord

  std::vector<uint64_t> PackAMMMWord(bdpars::MemId AM_or_MM, const std::vector<uint64_t> &payload) const;

  /// Core logic for all Dump calls
  VFieldValues DumpMem(unsigned int core_id, bdpars::MemId mem_id);
  /// Core logic for all Set calls
  void SetMem(
      unsigned int core_id,
      bdpars::MemId mem_id,
      const VFieldValues &data,
      unsigned int start_addr);

  ////////////////////////////////
  // register programming helpers

  void SetRegister(unsigned int core_id, bdpars::RegId reg_id, const FieldValues &field_vals);
  void SetToggle(unsigned int core_id, bdpars::RegId toggle_id, bool traffic_enable, bool dump_enable);
  bool SetToggleTraffic(unsigned int core_id, bdpars::RegId reg_id, bool en);
  bool SetToggleDump(unsigned int core_id, bdpars::RegId reg_id, bool en);

  /// Set memory delay line value
  void SetMemoryDelay(unsigned int core_id, bdpars::MemId mem_id, unsigned int read_value, unsigned int write_value);

  std::vector<Tag> GetFIFODump(unsigned int core_id, bdpars::OutputId output_id, unsigned int n_tags);

};

}  // bddriver namespace
}  // pystorm namespace

#endif
