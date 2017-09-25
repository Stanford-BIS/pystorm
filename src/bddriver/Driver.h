#ifndef DRIVER_H
#define DRIVER_H

#include <unordered_map>
#include <vector>
#include <functional>     // std::bind

#include "comm/Comm.h"
#include "common/BDPars.h"
#include "common/BDWord.h"
#include "common/BDState.h"
#include "common/MutexBuffer.h"
#include "decoder/Decoder.h"
#include "encoder/Encoder.h"

/*
 * TODO LIST: funnel/horn leaves that still need their calls finished
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
///                              (user/HAL)
///
///  ---[fns]--[fns]--[fns]----------------------[fns]-----------------------[fns]----  API
///       |      |      |            |             A                           A
///       V      V      V            |             |                           |
///  [private fns, e.g. PackWords]   |        [XXXX private fns, e.g. UnpackWords XXXX]
///          |        |              |             A                           A
///          V        V           [BDState]        |                           |
///   [MutexBuffer:enc_buf_in_]      |      [M.B.:dec_buf_out_[0]]    [M.B.:dec_buf_out_[0]] ...
///              |                   |                   A                   A
///              |                   |                   |                   |
///   ----------------------------[BDPars]------------------------------------------- funnel/horn payloads,
///              |                   |                   |                   |           organized by leaf
///              V                   |                   |                   |
///      [Encoder:encoder_]          |        [XXXXXXXXXXXX Decoder:decoder_ XXXXXXXXXX]
///              |                   |                        A
///              V                   |                        |
///   [MutexBuffer:enc_buf_out_]     |           [MutexBuffer:dec_buf_in_]
///              |                   |                      A
///              |                   |                      |
///  --------------------------------------------------------------------------------- raw data
///              |                                          |
///              V                                          |
///         [XXXXXXXXXXXXXXXXXXXX Comm:comm_ XXXXXXXXXXXXXXXXXXXX]
///                               |      A
///                               V      |
///  --------------------------------------------------------------------------------- USB
///
///                              (Braindrop)
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

  ////////////////////////////////////////////////////////////////////////////
  // Start/Stop/Init
  ////////////////////////////////////////////////////////////////////////////

  /// starts child workers, e.g. encoder and decoder
  void Start();
  /// stops the child workers
  void Stop();
  /// Initializes hardware state
  /// Calls Flush immediately
  void InitBD();
  /// Clears BD FIFOs
  /// Calls Flush immediately
  void InitFIFO(unsigned int core_id);

  ////////////////////////////////////////////////////////////////////////////
  // Traffic Control
  ////////////////////////////////////////////////////////////////////////////
  
  /// Flush queued up downstream traffic
  /// Commits queued-up messages (sends enough nops to flush the USB)
  /// By default, many configuration calls will call Flush()
  /// Notably, the Neuron config calls do not call Flush()
  void Flush();

  /// Control tag traffic
  void SetTagTrafficState(unsigned int core_id, bool en, bool flush=true);

  /// Control spike traffic from neuron array to datapath
  void SetSpikeTrafficState(unsigned int core_id, bool en, bool flush=true);
  /// Control spike traffic from neuron array to driver
  void SetSpikeDumpState(unsigned int core_id, bool en, bool flush=true);

  ////////////////////////////////////////////////////////////////////////////
  // ADC/DAC Config
  ////////////////////////////////////////////////////////////////////////////

  /// Program DAC value
  void SetDACValue(unsigned int core_id, bdpars::BDHornEP signal_id, unsigned int value, bool flush=true);
  /// Make DAC-to-ADC connection for calibration for a particular DAC
  void SetDACtoADCConnectionState(unsigned int core_id, bdpars::BDHornEP dac_signal_id, bool en, bool flush=true);

  /// Set large/small current scale for either ADC
  void SetADCScale(unsigned int core_id, bool adc_id, const std::string &small_or_large);  // XXX not implemented
  /// Turn ADC output on
  void SetADCTrafficState(unsigned int core_id, bool en);  // XXX not implemented

  ////////////////////////////////////////////////////////////////////////////
  // Neuron controls
  ////////////////////////////////////////////////////////////////////////////
  template<class U>
    void SetConfigMemory(unsigned int core_id, unsigned int elem_id,
                         std::map<U, std::vector<unsigned int>> config_map,
                         U config_type, unsigned int config_value);
    
  ////////////////////////////////////////////////////////////////////////////
  // Soma controls
  ////////////////////////////////////////////////////////////////////////////
  std::function<void(unsigned int, unsigned int, bdpars::ConfigSomaID, unsigned int)> SetSomaConfigMemory =
    std::bind(&Driver::SetConfigMemory<bdpars::ConfigSomaID>, this,
                std::placeholders::_1,
                std::placeholders::_2,
                config_soma_mem_,
                std::placeholders::_3,
                std::placeholders::_4
            );

  /// Enable/Disable Soma
  /// Map between memory and status
  ///     _KILL       Status
  ///       0         DISABLED
  ///       1         ENABLED
  void SetSomaEnableStatus(unsigned int core_id,
                           unsigned int soma_id,
                           bdpars::SomaStatusId status);

  /// Enable Soma
  std::function<void(unsigned int, unsigned int)> EnableSoma =
      std::bind(&Driver::SetSomaEnableStatus, this,
                std::placeholders::_1,
                std::placeholders::_2,
                bdpars::SomaStatusId::ENABLED);

  /// Disable Soma
  std::function<void(unsigned int, unsigned int)> DisableSoma =
      std::bind(&Driver::SetSomaEnableStatus, this,
                std::placeholders::_1,
                std::placeholders::_2,
                bdpars::SomaStatusId::DISABLED);

  /// Set Soma gain (post rectifier)
  /// Map between memory and gain values:
  ///     G<1>        G<0>        Gain
  ///      0           0          ONE_FOURTH (1/4)
  ///      0           1          ONE_THIRD (1/3)
  ///      1           0          ONE_HALF (1/2)
  ///      1           1          ONE (1)
  void SetSomaGain(unsigned int core_id, unsigned int soma_id,
                   bdpars::SomaGainId gain);

  /// Set offset sign (pre rectifier)
  /// Map between memory and sign
  ///     _ENPOSBIAS  Sign
  ///       0         POSITIVE
  ///       1         NEGATIVE
  void SetSomaOffsetSign(unsigned int core_id, unsigned int soma_id,
                         bdpars::SomaOffsetSignId offset_sign);

  /// Set Soma offset gain (pre rectifier)
  /// Map between memory and gain values:
  ///     B<1>        B<0>        Gain
  ///      0           0          ZERO (0)
  ///      0           1          ONE (1)
  ///      1           0          TWO (2)
  ///      1           1          THREE (3)
  void SetSomaOffsetMultiplier(unsigned int core_id, unsigned int soma_id,
                               bdpars::SomaOffsetMultiplierId soma_offset_multiplier);

  ////////////////////////////////////////////////////////////////////////////
  // Synapse controls
  ////////////////////////////////////////////////////////////////////////////
  std::function<void(unsigned int, unsigned int, bdpars::ConfigSynapseID, unsigned int)> SetSynapseConfigMemory =
    std::bind(&Driver::SetConfigMemory<bdpars::ConfigSynapseID>, this,
                std::placeholders::_1,
                std::placeholders::_2,
                config_synapse_mem_,
                std::placeholders::_3,
                std::placeholders::_4
            );


  /// Enable/Disable Synapse
  /// Map between memory and status
  ///     KILL        Status
  ///       0         ENABLED
  ///       1         DISABLED
  void SetSynapseEnableStatus(unsigned int core_id, unsigned int synapse_id,
                              bdpars::SynapseStatusId synapse_status);

  /// Enable Synapse
  std::function<void(unsigned int, unsigned int)> EnableSynapse =
      std::bind(&Driver::SetSynapseEnableStatus, this,
                std::placeholders::_1,
                std::placeholders::_2,
                bdpars::SynapseStatusId::ENABLED);

  /// Disable Synapse
  std::function<void(unsigned int, unsigned int)> DisableSynapse =
      std::bind(&Driver::SetSynapseEnableStatus, this,
                std::placeholders::_1,
                std::placeholders::_2,
                bdpars::SynapseStatusId::DISABLED);

  /// Enable/Disable Synapse ADC
  /// Map between memory and status
  ///     _ADC        Status
  ///       0         ENABLED
  ///       1         DISABLED
  void SetSynapseADCStatus(unsigned int core_id, unsigned int synapse_id,
                           bdpars::SynapseStatusId synapse_status);

  /// Enable Synapse ADC
  std::function<void(unsigned int, unsigned int)> EnableSynapseADC =
      std::bind(&Driver::SetSynapseADCStatus, this,
                std::placeholders::_1,
                std::placeholders::_2,
                bdpars::SynapseStatusId::ENABLED);

  /// Disable Synapse ADC
  std::function<void(unsigned int, unsigned int)> DisableSynapseADC =
      std::bind(&Driver::SetSynapseADCStatus, this,
                std::placeholders::_1,
                std::placeholders::_2,
                bdpars::SynapseStatusId::DISABLED);

  //////////////////////////////////////////////////////////////////////////
  // Diffusor controls
  //////////////////////////////////////////////////////////////////////////
  ///
  /// Set diffusor cuts' status.
  /// There are four possible cuts per tile:
  ///     * NORTH_LEFT
  ///     * NORTH_RIGHT
  ///     * WEST_TOP
  ///     * WEST_BOTTOM
  ///
  /// Map between memory and status
  ///     DIFF_CUT   Status
  ///       0         CLOSE
  ///       1         OPEN
  std::function<void(unsigned int, unsigned int, bdpars::DiffusorCutLocationId, unsigned int)> SetDiffusorConfigMemory =
    std::bind(&Driver::SetConfigMemory<bdpars::DiffusorCutLocationId>, this,
                std::placeholders::_1,
                std::placeholders::_2,
                config_diff_cut_mem_,
                std::placeholders::_3,
                std::placeholders::_4
            );

  void SetDiffusorCutStatus(unsigned int core_id,
                            unsigned int tile_id,
                            bdpars::DiffusorCutLocationId cut_id,
                            bdpars::DiffusorCutStatusId status);

  /// Open Diffusor cut
  std::function<void(unsigned int, unsigned int, bdpars::DiffusorCutLocationId)> OpenDiffusorCut =
      std::bind(&Driver::SetDiffusorCutStatus, this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                bdpars::DiffusorCutStatusId::OPEN);
  /// Close Diffusor cut
  std::function<void(unsigned int, unsigned int, bdpars::DiffusorCutLocationId)> CloseDiffusorCut =
      std::bind(&Driver::SetDiffusorCutStatus, this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                bdpars::DiffusorCutStatusId::CLOSE);

  /// Set all the diffusor cuts' status for a tile
  void SetDiffusorAllCutsStatus(unsigned int core_id, unsigned int tile_id,
      bdpars::DiffusorCutStatusId status);

  /// Open all diffusor cuts for a tile
  std::function<void(unsigned int, unsigned int)> OpenDiffusorAllCuts =
      std::bind(&Driver::SetDiffusorAllCutsStatus, this,
                std::placeholders::_1,
                std::placeholders::_2,
                bdpars::DiffusorCutStatusId::OPEN);

  /// Close all diffusor cuts for a tile
  std::function<void(unsigned int, unsigned int)> CloseDiffusorAllCuts =
      std::bind(&Driver::SetDiffusorAllCutsStatus, this,
                std::placeholders::_1,
                std::placeholders::_2,
                bdpars::DiffusorCutStatusId::CLOSE);

  //////////////////////////////////////////////////////////////////////////
  // Memory programming
  //////////////////////////////////////////////////////////////////////////
  
  /// Set memory delay line value
  void SetMemoryDelay(unsigned int core_id, bdpars::BDMemId mem_id, unsigned int read_value, unsigned int write_value, bool flush=true);

  /// Program a memory.
  /// BDWords must be constructed as the correct word type for the mem_id
  void SetMem(
      unsigned int core_id,
      bdpars::BDMemId mem_id,
      const std::vector<BDWord> &data,
      unsigned int start_addr);

  /// Dump the contents of one of the memories.
  /// BDWords must subsequently be unpacked as the correct word type for the mem_id
  std::vector<BDWord> DumpMem(unsigned int core_id, bdpars::BDMemId mem_id);

  /// Dump copy of traffic pre-FIFO
  void SetPreFIFODumpState(unsigned int core_id, bool dump_en);
  /// Dump copy of traffic post-FIFO, tag msbs = 0
  void SetPostFIFODumpState(unsigned int core_id, bool dump_en);

  /// Get pre-FIFO tags recorded during dump
  std::vector<BDWord> GetPreFIFODump(unsigned int core_id);
  /// Get post-FIFO tags recorded during dump
  std::pair<std::vector<BDWord>, std::vector<BDWord> > GetPostFIFODump(unsigned int core_id);

  /// Get warning count
  std::pair<unsigned int, unsigned int> GetFIFOOverflowCounts(unsigned int core_id);

  //////////////////////////////////////////////////////////////////////////
  // Spike/Tag Streams
  //////////////////////////////////////////////////////////////////////////

  /// Send a stream of spikes to neurons
  void SendSpikes(
      unsigned int core_id,
      const std::vector<BDWord>& spikes,
      const std::vector<BDTime> times,
      bool flush=true);

  /// Receive a stream of spikes
  std::pair<std::vector<BDWord>,
            std::vector<BDTime> > RecvSpikes(unsigned int core_id) {
    return RecvFromEP(core_id, bdpars::BDFunnelEP::NRNI);
  }

  /// Send a stream of tags
  void SendTags(
      unsigned int core_id,
      const std::vector<BDWord>& tags,
      const std::vector<BDTime> times,
      bool flush=true);

  /// Receive a stream of tags
  /// receive from both tag output leaves, the Acc and TAT
  std::pair<std::vector<BDWord>,
            std::vector<BDTime>> RecvTags(unsigned int core_id);

  //////////////////////////////////////////////////////////////////////////
  // BDState queries
  //////////////////////////////////////////////////////////////////////////

  // XXX note that these queries are NOT subject to timing assumptions!
  // this is the SOFTWARE state of the board
  // for any purpose where a timing assumption has some functional importance,
  // there is a separate call, e.g. for traffic registers

  /// Get register contents by name.
  inline const std::pair<const BDWord, bool> GetRegState(
      unsigned int core_id, bdpars::BDHornEP reg_id) const {
    return bd_state_[core_id].GetReg(reg_id);
  }

  /// Get software state of memory contents: this DOES NOT dump the memory.
  inline const std::vector<BDWord> *GetMemState(bdpars::BDMemId mem_id, unsigned int core_id) const { return bd_state_[core_id].GetMem(mem_id); }

 protected:

  ////////////////////////////////
  // data members

  /// parameters describing parameters of the software (e.g. buffer depths)
  const driverpars::DriverPars *driver_pars_;
  /// parameters describing BD hardware
  const bdpars::BDPars *bd_pars_;
  /// best-of-driver's-knowledge state of bd hardware
  std::vector<BDState> bd_state_;

  // downstream buffers
  
  // queue for sequenced, but un-timed traffic
  std::queue<std::unique_ptr<std::vector<EncInput>>> sequenced_queue_;
 
  // priority queue for timed traffic
  std::vector<EncInput> timed_queue_;

  /// thread-safe, MPMC buffer between breadth of downstream driver API and the encoder
  MutexBuffer<EncInput> *enc_buf_in_;
  /// thread-safe, MPMC buffer between the encoder and comm
  MutexBuffer<EncOutput> *enc_buf_out_;

  // upstream buffers

  /// thread-safe, MPMC buffer between comm and decoder
  MutexBuffer<DecInput> *dec_buf_in_;

  /// vector of thread-safe, MPMC buffers between decoder and breadth of upstream driver API
  std::unordered_map<uint8_t, MutexBuffer<DecOutput> *> dec_bufs_out_;

  /// deserializers for upstream traffic
  std::unordered_map<uint8_t, VectorDeserializer<DecOutput>> up_ep_deserializers_;

  /// encodes traffic to BD
  Encoder *enc_;
  /// decodes traffic from BD
  Decoder *dec_;

  /// comm module talks to libUSB or to file
  comm::Comm *comm_;

  ////////////////////////////////
  // traffic helpers
  const std::vector<bdpars::BDHornEP> kTrafficRegs = {
      bdpars::BDHornEP::NEURON_DUMP_TOGGLE,
      bdpars::BDHornEP::TOGGLE_PRE_FIFO,
      bdpars::BDHornEP::TOGGLE_POST_FIFO0,
      bdpars::BDHornEP::TOGGLE_POST_FIFO1};

  std::unordered_map<unsigned int, std::vector<bool> > last_traffic_state_;

  /// Stops traffic for a core and saves the previous state in last_traffic_state_
  /// Calls Flush()
  void PauseTraffic(unsigned int core_id);
  void ResumeTraffic(unsigned int core_id);

  ////////////////////////////////
  // Main send/recv calls

  /// Sends a vector of payloads to a single <core_id> and <leaf_id>.
  /// Pushes contents into one of the priority queues. A Flush() is needed to commit the traffic.
  /// times = {} is a special code, sends to the sequenced_queue_,
  /// otherwise, sends to timed_queue_
  void SendToEP(unsigned int core_id, uint8_t ep_code, const std::vector<BDWord> &payload, const std::vector<BDTime> &times = {});
  // can call SendToEP on BDHornEP, FPGARegEP, FPGAChannelEP instead of uint8_t ep_code
  template <class T>
  void SendToEP(unsigned int core_id, T ep, const std::vector<BDWord> &payload, const std::vector<BDTime> &times = {}) {
    SendToEP(core_id, bd_pars_->DnEPCodeFor(ep), payload, times);
  }

  /// Pops from a <leaf_id>'s dec_bufs_out_[] <num_to_recv> payloads
  /// associated with a supplied <core_id>.
  ///
  /// If <num_to_recv> is zero, then receive whatever's currently in the buffer.
  /// Returns vector of deserialized payloads. For payloads that don't need to be deserialized,
  /// that might come from multiple cores, or that need time_epoch information,
  /// This isn't the most effective call.
  std::pair<std::vector<BDWord>,
            std::vector<BDTime>>
    RecvFromEP(unsigned int core_id, uint8_t ep_code, unsigned int timeout_us=0);

  /// Wrapper for convenience, can call with BDFunnelEP or FPGAOutputEP
  template <class T>
  std::pair<std::vector<BDWord>,
            std::vector<BDTime>>
    RecvFromEP(unsigned int core_id, T ep_enum, unsigned int timeout_us=0) { return RecvFromEP(core_id, bd_pars_->UpEPCodeFor(ep_enum), timeout_us); }

  ////////////////////////////////
  // memory programming helpers

  template <class TWrite>
  std::vector<BDWord> PackRWProgWords(const std::vector<BDWord>& payload, unsigned int start_addr) const;
  template <class TRead>
  std::vector<BDWord> PackRWDumpWords(unsigned int start_addr, unsigned int end_addr) const;

  template <class TSet, class TWrite>
  std::vector<BDWord> PackRIWIProgWords( const std::vector<BDWord> &payload, unsigned int start_addr) const;

  template <class TSet, class TRead>
  std::vector<BDWord> PackRIWIDumpWords(unsigned int start_addr, unsigned int end_addr) const;

  template <class TSet, class TReadWrite, class TInc>
  std::vector<BDWord> PackRMWProgWords(const std::vector<BDWord> &payload, unsigned int start_addr) const;
  // note: RMWProgWord can function as RMWDumpWord

  template <class AMorMMEncapsulation>
  std::vector<BDWord> PackAMMMWord(const std::vector<BDWord> &payload) const;


  ////////////////////////////////
  // register programming helpers

  void SetBDRegister(unsigned int core_id, bdpars::BDHornEP reg_id, BDWord word, bool flush=true);
  void SetToggle(unsigned int core_id, bdpars::BDHornEP toggle_id, bool traffic_enable, bool dump_enable, bool flush=true);
  bool SetToggleTraffic(unsigned int core_id, bdpars::BDHornEP reg_id, bool en, bool flush=true);
  bool SetToggleDump(unsigned int core_id, bdpars::BDHornEP reg_id, bool en, bool flush=true);


  ///////////////////////////////
  // Neuron config stuff

  /// Config memory map
  /// Soma configuration bits for 16 Somas in a tile.
  static std::map<bdpars::ConfigSomaID, std::vector<unsigned int>> config_soma_mem_;

  /// Synapse configuration bits for 4 Synapses in a tile.
  static std::map<bdpars::ConfigSynapseID, std::vector<unsigned int>> config_synapse_mem_;

  /// Diffusor cut 'enable' memory config.
  /// Setting 1 cuts the diffusor at the location.
  static std::map<bdpars::DiffusorCutLocationId, std::vector<unsigned int>> config_diff_cut_mem_;
};

}  // bddriver namespace
}  // pystorm namespace

#endif
