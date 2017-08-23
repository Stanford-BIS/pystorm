#ifndef DRIVER_H
#define DRIVER_H

#include <unordered_map>
#include <vector>
#include <functional>

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

  ////////////////////////////////////////////////////////////////////////////
  // Neuron controls
  ////////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////////
  // Soma controls
  ////////////////////////////////////////////////////////////////////////////

  /// Enable/Disable Soma
  /// Map between memory and status
  ///     _KILL       Status
  ///       0         DISABLED
  ///       1         ENABLED
  void SetSomaEnableStatus(unsigned int core_id,
                           unsigned int soma_id,
                           bdpars::SomaStatusId soma_status);

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
                   bdpars::SomaGainId soma_gain);

  /// Set offset sign (pre rectifier)
  /// Map between memory and sign
  ///     _ENPOSBIAS  Sign
  ///       0         POSITIVE
  ///       1         NEGATIVE
  void SetSomaOffsetSign(unsigned int core_id, unsigned int soma_id,
                         bdpars::SomaOffsetSignId soma_offset_sign);

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


  ////////////////////////////////
  // memory programming

  /// Program a memory.
  /// BDWords must be constructed as the correct word type for the mem_id
  void SetMem(
      unsigned int core_id,
      bdpars::MemId mem_id,
      const std::vector<BDWord> &data,
      unsigned int start_addr);

  /// Dump the contents of one of the memories.
  /// BDWords must subsequently be unpacked as the correct word type for the mem_id
  std::vector<BDWord> DumpMem(unsigned int core_id, bdpars::MemId mem_id);

  /// Dump copy of traffic pre-FIFO
  void SetPreFIFODumpState(unsigned int core_id, bool dump_en);
  /// Dump copy of traffic post-FIFO, tag msbs = 0
  void SetPostFIFODumpState(unsigned int core_id, bool dump_en);

  /// Get pre-FIFO tags recorded during dump
  std::vector<BDWord> GetPreFIFODump(unsigned int core_id, unsigned int n_tags);
  /// Get post-FIFO tags recorded during dump
  std::pair<std::vector<BDWord>, std::vector<BDWord> > GetPostFIFODump(unsigned int core_id, unsigned int n_tags0, unsigned int n_tags1);

  /// Get warning count
  std::pair<unsigned int, unsigned int> GetFIFOOverflowCounts(unsigned int core_id);

  ////////////////////////////////
  // Spike/Tag Streams

  /// Send a stream of spikes to neurons
  void SendSpikes(
      const std::vector<unsigned int>& core_ids,
      const std::vector<BDWord>& spikes,
      const std::vector<BDTime> times);

  /// Receive a stream of spikes
  std::tuple<std::vector<unsigned int>,
          std::vector<BDWord>,
          std::vector<BDTime> > RecvSpikes(unsigned int max_to_recv);

  /// Send a stream of tags
  void SendTags(
      const std::vector<unsigned int>& core_ids,
      const std::vector<BDWord>& tags,
      const std::vector<BDTime> times);

  /// Receive a stream of tags
  /// receive from both tag output leaves, the Acc and TAT
  /// Use TATOutputTags to unpack
  std::tuple<std::vector<unsigned int>,
          std::vector<BDWord>,
          std::vector<BDTime> > RecvTags(unsigned int max_to_recv);

  ////////////////////////////////
  // BDState queries

  // XXX note that these queries are NOT subject to timing assumptions!
  // this is the SOFTWARE state of the board
  // for any purpose where a timing assumption has some functional importance,
  // there is a separate call, e.g. for traffic registers

  /// Get register contents by name.
  inline const std::pair<const BDWord *, bool> GetRegState(
      unsigned int core_id, bdpars::RegId reg_id) const {
    return bd_state_[core_id].GetReg(reg_id);
  }

  /// Get software state of memory contents: this DOES NOT dump the memory.
  inline const std::vector<BDWord> *GetMemState(bdpars::MemId mem_id, unsigned int core_id) const { return bd_state_[core_id].GetMem(mem_id); }

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
  void SendToHorn(unsigned int core_id, bdpars::HornLeafId leaf_id, const std::vector<BDWord> &payload);

  /// Pops from a <leaf_id>'s dec_bufs_out_[] <num_to_recv> payloads
  /// associated with a supplied <core_id>.
  ///
  /// If <num_to_recv> is zero, then receive whatever's currently in the buffer.
  /// Returns vector of deserialized payloads. For payloads that don't need to be deserialized,
  /// that might come from multiple cores, or that need time_epoch information,
  /// This isn't the most effective call.
  std::vector<BDWord> RecvFromFunnel(
      unsigned int core_id,
      bdpars::FunnelLeafId leaf_id,
      unsigned int num_to_recv = 0);

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

  void SetRegister(unsigned int core_id, bdpars::RegId reg_id, BDWord word);
  void SetToggle(unsigned int core_id, bdpars::RegId toggle_id, bool traffic_enable, bool dump_enable);
  bool SetToggleTraffic(unsigned int core_id, bdpars::RegId reg_id, bool en);
  bool SetToggleDump(unsigned int core_id, bdpars::RegId reg_id, bool en);

  /// Set memory delay line value
  void SetMemoryDelay(unsigned int core_id, bdpars::MemId mem_id, unsigned int read_value, unsigned int write_value);

  std::vector<BDWord> GetFIFODump(unsigned int core_id, bdpars::OutputId output_id, unsigned int n_tags);

  /// Config memory map
  /// Soma configuration bits for 16 Somas in a tile.
  std::map<bdpars::ConfigSomaID, std::vector<unsigned int>> config_soma_mem_ {
    {bdpars::ConfigSomaID::GAIN_0, {112, 114, 82, 80, 119, 117, 85, 87, 55, 53, 21, 23, 48, 50, 18, 16}},
    {bdpars::ConfigSomaID::GAIN_1, {104, 97, 65, 72, 111, 102, 70, 79, 47, 38, 6, 15, 40, 33, 1, 8}},
    {bdpars::ConfigSomaID::OFFSET_0, {113, 115, 83, 81, 118, 116, 84, 86, 54, 52, 20, 22, 49, 51, 19, 17}},
    {bdpars::ConfigSomaID::OFFSET_1, {120, 122, 90, 88, 127, 125, 93, 95, 63, 61, 29, 31, 56, 58, 26, 24}},
    {bdpars::ConfigSomaID::ENABLE, {121, 123, 91, 89, 126, 124, 92, 94, 62, 60, 28, 30, 57, 59, 27, 25}},
    {bdpars::ConfigSomaID::SUBTRACT_OFFSET, {96, 106, 74, 64, 103, 109, 77, 71, 39, 45, 13, 7, 32, 42, 10, 0}},
  };

  /// Synapse configuration bits for 4 Synapses in a tile.
  std::map<bdpars::ConfigSynapseID, std::vector<unsigned int>> config_synapse_mem_ {
    {bdpars::ConfigSynapseID::SYN_DISABLE, {75, 76, 12, 11}},
    {bdpars::ConfigSynapseID::ADC_DISABLE, {67, 68, 4, 3}}
  };

  /// Diffusor cut 'enable' memory config.
  /// Setting 1 cuts the diffusor at the location.
  std::map<bdpars::DiffusorCutLocationId, unsigned int> config_diff_cut_mem_{
    {bdpars::DiffusorCutLocationId::NORTH_LEFT, 99},
    {bdpars::DiffusorCutLocationId::NORTH_RIGHT, 100},
    {bdpars::DiffusorCutLocationId::WEST_TOP, 107},
    {bdpars::DiffusorCutLocationId::WEST_BOTTOM, 43},
  };
};

}  // bddriver namespace
}  // pystorm namespace

#endif
