#ifndef DRIVER_H
#define DRIVER_H

#include <unordered_map>
#include <vector>
#include <chrono>
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

// Opal Kelly parameters
static const std::string OK_BITFILE = "OK_BITFILE.bit";
static const std::string OK_SERIAL = "";
struct OKPars {
    OKPars() : ok_bitfile(OK_BITFILE), ok_serial(OK_SERIAL) {}
    std::string ok_bitfile;
    std::string ok_serial;
};

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
  inline const BDState *GetState(unsigned int core_id) { return &bd_state_[core_id]; }

  void testcall(const std::string &msg);

  ////////////////////////////////////////////////////////////////////////////
  // Start/Stop/Init
  ////////////////////////////////////////////////////////////////////////////

  /// starts child workers, e.g. encoder and decoder
  /// returns 0 if successful, -1 if comm init fails
  int Start();
  /// stops the child workers
  void Stop();

  /// Sets the FPGA time resolution (also is the interval that FPGA updates SG values)
  void SetTimeUnitLen(BDTime ns_per_unit);
  /// Sets how long the FPGA waits between sending upstream HBs (and updating/reporting SF values)
  void SetTimePerUpHB(BDTime ns_per_hb);
  /// Sets the FPGA's clock to 0, also resets driver time
  void ResetFPGATime();
  /// Get the most recently received upstream FPGA clock value
  BDTime GetFPGATime();
  /// Get the most recently received upstream FPGA clock value in seconds
  float GetFPGATimeSec(){
      return static_cast<float>(GetFPGATime()) * 1e-9;
  }
  /// Returns driver (PC) time in ns
  BDTime GetDriverTime() const;
  // Set the Opal Kelly bitfile location
  void SetOKBitFile(std::string bitfile);
  /// Cycles BD pReset/sReset
  /// Useful for testing, but leaves memories in an indeterminate state
  void ResetBD();
  /// Clears BD FIFOs
  /// Calls Flush immediately
  void InitFIFO(unsigned int core_id);
  /// Inits the DACs to default values
  void InitDAC(unsigned int core_id, bool flush=true);
  /// Initializes BD hardware state
  /// Calls Reset, InitFIFO, among other things
  /// Calls Flush immediately
  void InitBD();
  /// Initializes FPGA state
  void InitFPGA();
  /// Empties all driver output queues
  void ClearOutputs();
  
  ////////////////////////////////////////////////////////////////////////////
  // AER Address <-> Y,X mapping static member fns
  ////////////////////////////////////////////////////////////////////////////
  
  /// Given flat xy_addr (addr scan along x then y) config memory (16-neuron tile) address, get AER address
  unsigned int GetMemAERAddr(unsigned int xy_addr) const                             { return bd_pars_->mem_xy_to_aer_.at(xy_addr); }
  /// Given x, y config memory (16-neuron tile) address, get AER address
  unsigned int GetMemAERAddr(unsigned int x, unsigned int y) const                   { return GetMemAERAddr(y*16 + x); }
  /// Given flat xy_addr (addr scan along x then y) synapse address, get AER address
  unsigned int GetSynAERAddr(unsigned int xy_addr) const                             { return bd_pars_->syn_xy_to_aer_.at(xy_addr); }
  /// Given x, y synapse address, get AER address
  unsigned int GetSynAERAddr(unsigned int x, unsigned int y) const                   { return GetSynAERAddr(y*32 + x); }
  /// Given flat xy_addr soma address, get AER address
  unsigned int GetSomaAERAddr(unsigned int xy_addr) const                            { return bd_pars_->soma_xy_to_aer_.at(xy_addr); }
  /// Given x, y soma address, get AER address
  unsigned int GetSomaAERAddr(unsigned int x, unsigned int y) const                  { return GetSomaAERAddr(y*64 + x); }
  /// Given AER synapse address, get flat xy_addr (addr scan along x then y)
  unsigned int GetSomaXYAddr(unsigned int aer_addr) const                            { return bd_pars_->soma_aer_to_xy_.at(aer_addr); }

  /// Utility function to process spikes a little more quickly
  std::vector<unsigned int> GetSomaXYAddrs(const std::vector<unsigned int>& aer_addrs) const {
    std::vector<unsigned int> to_return(aer_addrs.size());;
    for (unsigned int i = 0; i < aer_addrs.size(); i++) {
      unsigned int addr = aer_addrs[i];
      if (addr < 4096) {
        to_return[i] = bd_pars_->soma_aer_to_xy_.at(addr);
      } else {
        cout << "WARNING: supplied bad AER addr to GetSomaXYAddrs: " << addr << endl;
      }
    }
    return to_return;
  }

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
  /// DAC value is from 1 to 1024
  void SetDACCount(unsigned int core_id, bdpars::BDHornEP signal_id, unsigned int value, bool flush=true);
  // Specify any `value` < 0.0 to use default value;
  void SetDACValue(unsigned int core_id, bdpars::BDHornEP signal_id, float value, bool flush=true);

  unsigned int GetDACCurrentCount(unsigned int core_id, bdpars::BDHornEP signal_id);

  unsigned int GetDACScaling(bdpars::BDHornEP signal_id);

  unsigned int GetDACDefaultCount(bdpars::BDHornEP signal_id);

  float GetDACUnitCurrent(bdpars::BDHornEP signal_id);

  /// Make DAC-to-ADC connection for calibration for a particular DAC
  void SetDACtoADCConnectionState(unsigned int core_id, bdpars::BDHornEP dac_signal_id, bool en, bool flush=true);

  /// Set large/small current scale for either ADC
  void SetADCScale(unsigned int core_id, unsigned int adc_id, const std::string &small_or_large);  // XXX not implemented
  /// Turn ADC output on
  void SetADCTrafficState(unsigned int core_id, bool en);  // XXX not implemented

  ////////////////////////////////////////////////////////////////////////////
  // Neuron controls
  ////////////////////////////////////////////////////////////////////////////
  template<class U>
    void SetConfigMemory(unsigned int core_id, unsigned int elem_id,
                         std::unordered_map<U, std::vector<unsigned int>> config_map,
                         U config_type, bool config_value);

  ////////////////////////////////////////////////////////////////////////////
  // Soma controls
  ////////////////////////////////////////////////////////////////////////////
  std::function<void(unsigned int, unsigned int, bdpars::ConfigSomaID, bool)> SetSomaConfigMemory =
    std::bind(&Driver::SetConfigMemory<bdpars::ConfigSomaID>, this,
                std::placeholders::_1,
                std::placeholders::_2,
                bd_pars_->config_soma_mem_,
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

  /// Enable Soma in XY space
  void EnableSomaXY(unsigned int core_id, unsigned int x, unsigned int y) {
    unsigned int AER_addr = GetSomaAERAddr(x, y);
    EnableSoma(core_id, AER_addr);
  }

  /// Disable Soma in XY space
  void DisableSomaXY(unsigned int core_id, unsigned int x, unsigned int y) {
    unsigned int AER_addr = GetSomaAERAddr(x, y);
    DisableSoma(core_id, AER_addr);
  }

  /// Set Soma gain in XY space
  void SetSomaGainXY(unsigned int core_id, unsigned int x, unsigned int y, bdpars::SomaGainId gain) {
    unsigned int AER_addr = GetSomaAERAddr(x, y);
    SetSomaGain(core_id, AER_addr, gain);
  }

  /// Set Soma offset sign in XY space
  void SetSomaOffsetSignXY(unsigned int core_id, unsigned int x, unsigned int y, bdpars::SomaOffsetSignId sign) {
    unsigned int AER_addr = GetSomaAERAddr(x, y);
    SetSomaOffsetSign(core_id, AER_addr, sign);
  }

  /// Set Soma offset multiplier in XY space
  void SetSomaOffsetMultiplierXY(unsigned int core_id, unsigned int x, unsigned int y, bdpars::SomaOffsetMultiplierId multiplier) {
    unsigned int AER_addr = GetSomaAERAddr(x, y);
    SetSomaOffsetMultiplier(core_id, AER_addr, multiplier);
  }
  

  ////////////////////////////////////////////////////////////////////////////
  // Synapse controls
  ////////////////////////////////////////////////////////////////////////////
  std::function<void(unsigned int, unsigned int, bdpars::ConfigSynapseID, bool)> SetSynapseConfigMemory =
    std::bind(&Driver::SetConfigMemory<bdpars::ConfigSynapseID>, this,
                std::placeholders::_1,
                std::placeholders::_2,
                bd_pars_->config_synapse_mem_,
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

  void EnableSynapseXY(unsigned int core_id, unsigned int x, unsigned int y) {
    unsigned int AER_addr = GetSynAERAddr(x, y);
    EnableSynapse(core_id, AER_addr);
  }

  void DisableSynapseXY(unsigned int core_id, unsigned int x, unsigned int y) {
    unsigned int AER_addr = GetSynAERAddr(x, y);
    DisableSynapse(core_id, AER_addr);
  }
  
  void EnableSynapseADCXY(unsigned int core_id, unsigned int x, unsigned int y) {
    unsigned int AER_addr = GetSynAERAddr(x, y);
    EnableSynapseADC(core_id, AER_addr);
  }

  void DisableSynapseADCXY(unsigned int core_id, unsigned int x, unsigned int y) {
    unsigned int AER_addr = GetSynAERAddr(x, y);
    DisableSynapseADC(core_id, AER_addr);
  }
    
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
  std::function<void(unsigned int, unsigned int, bdpars::DiffusorCutLocationId, bool)> SetDiffusorConfigMemory =
    std::bind(&Driver::SetConfigMemory<bdpars::DiffusorCutLocationId>, this,
                std::placeholders::_1,
                std::placeholders::_2,
                bd_pars_->config_diff_cut_mem_,
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

  // convenience functions to pack memory entries

  /// Pack PAT words
  ///
  /// inputs: three equal-length vectors. Each index corresponds to the fields of a single 
  /// PAT entry.
  ///
  /// Spikes leaving the neuron array index into the PAT for redirection to the accumulator.
  /// The accumulator takes in an AM and MM address (for its buckets and weights, respectively).
  ///
  /// Each 8x8 group of neurons shares the same PAT entry. The PAT entry contains:
  ///   AM_addr     : 10 bits,  an AM address
  ///   MM_addr_msb : 2 bits, msbs to use in computing the MM addr
  ///   MM_addr_lsb : 8 bits, lsbs to use in computing the MM addr
  ///
  /// basically, the PAT does (in pseudo-verilog):
  /// 
  /// function PAT(aer_addr) {
  ///   logic[5:0] aer_msb, aer_lsb
  ///   {aer_msb, aer_lsb} = aer_addr
  ///
  ///   logic[19:0] entry = PAT[aer_msb]
  ///   return (entry.AM_addr, {entry.MM_addr_msb, aer_lsb, entry.MM_addr_lsb})
  /// }
  ///
  /// The MM can be thought of as being 256x256 (64K total entries), comprised by
  /// four 64x256 "fat" blocks stacked on top of each other. The PAT MM MSBs 
  /// determine which fat block a 64-neuron group's decoders sit in and the MM LSBs
  /// determine which column the decoders start in (the accumulator walks along the X-axis).
  /// The neuron's sub-idx (aer_lsb) determines which row in the fat block is used by that neuron.
  std::vector<BDWord> PackPATWords(const std::vector<unsigned int>& AM_addrs,
                                   const std::vector<unsigned int>& MM_addrs_lsb,
                                   const std::vector<unsigned int>& MM_addrs_msb) {
    assert(AM_addrs.size() == MM_addrs_lsb.size());
    assert(AM_addrs.size() == MM_addrs_msb.size());
    std::vector<BDWord> packed(AM_addrs.size());
    for (unsigned int i = 0; i < AM_addrs.size(); i++) {
      packed[i] = PackWord<PATWord>({
        {PATWord::AM_ADDRESS, AM_addrs[i]},
        {PATWord::MM_ADDRESS_LO, MM_addrs_lsb[i]},
        {PATWord::MM_ADDRESS_HI, MM_addrs_msb[i]}});
    }
    return packed;
  }

  /// Pack AM words
  /// 
  /// inputs: three equal-length vectors. Each index corresponds to the fields of a single
  /// AM entry. 
  ///
  /// The AM works in tandem with the MM to feed the accumulator.
  /// When a spike or tag enters the accumulator with an AM or MM address,
  /// the accumulator will do as many reads of the accumulator and MM as there are
  /// dimensions in the output of the decode/transform being performed.
  /// Each AM entry therefore corresponds to the state of a single accumulator bucket.
  ///
  /// The AM entry has 4 fields:
  ///   value         : 19 bits, current bucket value
  ///   threshold_idx : 3 bits, determines the overflow value of the accumulator bucket.
  ///                   this value, in the same units as the MM weights is: 2**(6 + threshold_idx)
  ///                   making thresholds of 64 to 8192 possible. This is meant to optimize 
  ///                   the dynamic range of the decode weights.
  ///  stop           : 1 bit, 1 denotes this is the final bucket (last dimension) 
  ///                   of a decode/transform
  ///  output_tag     : 19 bits, the global tag to emit when the accumulator overflows
  ///
  /// (we don't program the value)
  /// 
  /// The accumulator does (roughly speaking):
  /// 
  /// function Accumulator(am_addr, mm_addr) {
  ///   stop = False
  ///   curr_am_addr = am_addr
  ///   curr_mm_addr = mm_addr
  ///   outputs = []
  ///   while (!stop) {
  ///     am_entry = AM[curr_am_addr]
  ///     mm_entry = MM[curr_mm_addr]
  ///     am_entry.value += mm_entry.weight
  ///     thr_val = 2**(6 + am_entry.threshold_idx);
  ///     if (am_entry.value >= thr_val) {
  ///       am_entry.value -= thr_val
  ///       outputs.append((am_entry.output_tag, +1))
  ///     }
  ///     if (am_entry.value <= -thr_val) {
  ///       am_entry.value += thr_val
  ///       outputs.append((am_entry.output_tag, -1))
  ///     }
  ///     curr_am_addr++
  ///     curr_mm_addr++
  ///   }
  ///   return outputs
  /// }
  std::vector<BDWord> PackAMWords(const std::vector<unsigned int>& threshold_idxs,
                                  const std::vector<unsigned int>& output_tags,
                                  const std::vector<unsigned int>& stops) {
    unsigned int size = stops.size();
    assert(size == threshold_idxs.size());
    assert(size == output_tags.size());
    std::vector<BDWord> packed(size);
    for (unsigned int i = 0; i < size; i++) {
      packed[i] = PackWord<AMWord>({
        {AMWord::ACCUMULATOR_VALUE, 0}, // could look up current value, clobber instead
        {AMWord::THRESHOLD, threshold_idxs[i]},
        {AMWord::STOP, stops[i]},
        {AMWord::NEXT_ADDRESS, output_tags[i]}});
    }
    return packed;
  }

  // (MM word has a single field, the weight, no need to pack)

  /// Packs TAT Spike Words
  ///
  /// inputs are four vectors. synapse_xs, synapse_ys, and synapse_signs must be length 2*N, 
  /// stops are length N. synapse_xs/ys/signs[2*i], synapse_xs/ys/signs[2*i+1],
  /// and stops[i] all correspond to the same TAT entry
  /// 
  /// The TAT takes input tags, uses it to index the memory, which it walks through
  /// until encountering a stop bit. For each entry, it does one of three things:
  ///   send spikes to two different synapses, optionally flipping the sign of each
  ///   emit a different global tag
  ///   send an input to the accumulator
  /// 
  /// for the spikes or accumulator outputs, if the count is greater than 1 or less than -1,
  /// a single set of outputs is produced (with the same sign), the count is 
  /// incremented or decremented, and the input is re-submitted to the FIFO. After leaving
  /// the FIFO, it will return to the TAT until the count is exhausted. 
  /// The resubmission to the FIFO has the effect of round-robinning between pending operations.
  ///
  /// A TAT Spike Word has 5 programmable fields:
  ///   stop              : 1 bit, whether or not this is the last entry for the input tag
  ///   synapse address 0 : 10 bits, AER address of the first synapse to hit
  ///   synapse sign 0    : 1 bit, whether to invert or not invert sign of input spikes 
  ///                       to first synapse, "0" means invert, "1" means don't invert
  ///   synapse address 1 : 10 bits, AER address of the second synapse to hit
  ///   synapse sign 1    : 1 bit, whether to invert or not invert sign of input spikes 
  ///                       to second synapse, "0" means invert, "1" means don't invert
  /// 
  /// note that you can't just hit 1 synapse per entry. You must hit 2.
  /// the synapse inputs are wired backwards. Hence the slightly confusing "1" for invert, 
  /// "0" for no inversion with the synapse signs
  std::vector<BDWord> PackTATSpikeWords(const std::vector<unsigned int>& synapse_xs,
                                        const std::vector<unsigned int>& synapse_ys,
                                        const std::vector<unsigned int>& synapse_signs,
                                        const std::vector<unsigned int>& stops) {
    
    unsigned int size = stops.size();
    assert(2*size == synapse_xs.size());
    assert(2*size == synapse_ys.size());
    assert(2*size == synapse_signs.size());

    std::vector<BDWord> packed(size);
    for (unsigned int i = 0; i < size; i++) {
      unsigned int addr0 = GetSynAERAddr(synapse_xs[2*i  ], synapse_ys[2*i  ]);
      unsigned int addr1 = GetSynAERAddr(synapse_xs[2*i+1], synapse_ys[2*i+1]);
      unsigned int sign0 = synapse_signs[2*i  ];
      unsigned int sign1 = synapse_signs[2*i+1];
      packed[i] = PackWord<TATSpikeWord>({
        {TATSpikeWord::STOP, stops[i]},
        {TATSpikeWord::SYNAPSE_ADDRESS_0, addr0},
        {TATSpikeWord::SYNAPSE_ADDRESS_1, addr1},
        {TATSpikeWord::SYNAPSE_SIGN_0, sign0},
        {TATSpikeWord::SYNAPSE_SIGN_1, sign1}});
    }
    return packed;
  }

  /// Packs TAT Tag Words
  ///
  /// inputs are three vectors of the same length. Each index corresponds to the same TAT entry
  /// 
  /// The TAT takes input tags, uses it to index the memory, which it walks through
  /// until encountering a stop bit. For each entry, it does one of three things:
  ///   send spikes to two different synapses, optionally flipping the sign of each
  ///   emit a different global tag
  ///   send an input to the accumulator
  /// 
  /// for the spikes or accumulator outputs, if the count is greater than 1 or less than -1,
  /// a single set of outputs is produced (with the same sign), the count is 
  /// incremented or decremented, and the input is re-submitted to the FIFO. After leaving
  /// the FIFO, it will return to the TAT until the count is exhausted. 
  /// The resubmission to the FIFO has the effect of round-robinning between pending operations.
  ///
  /// A TAT Tag Word has 3 programmable fields:
  ///   stop         : 1 bit, whether or not this is the last entry for the input tag
  ///   tag          : 11 bits, output tag to output
  ///   global route : 12 bits, global route to output
  std::vector<BDWord> PackTATTagWords(const std::vector<unsigned int>& tags,
                                      const std::vector<unsigned int>& global_routes,
                                      const std::vector<unsigned int>& stops) {
    
    unsigned int size = stops.size();
    assert(size == tags.size());
    assert(size == global_routes.size());

    std::vector<BDWord> packed(size);
    for (unsigned int i = 0; i < size; i++) {
      packed[i] = PackWord<TATTagWord>({
        {TATTagWord::STOP, stops[i]},
        {TATTagWord::TAG, tags[i]},
        {TATTagWord::GLOBAL_ROUTE, global_routes[i]}});
    }
    return packed;
  }

  /// Packs TAT Acc Words
  ///
  /// inputs are three vectors of the same length. Each index corresponds to the same TAT entry
  /// 
  /// The TAT takes input tags, uses it to index the memory, which it walks through
  /// until encountering a stop bit. For each entry, it does one of three things:
  ///   send spikes to two different synapses, optionally flipping the sign of each
  ///   emit a different global tag
  ///   send an input to the accumulator
  /// 
  /// for the spikes or accumulator outputs, if the count is greater than 1 or less than -1,
  /// a single set of outputs is produced (with the same sign), the count is 
  /// incremented or decremented, and the input is re-submitted to the FIFO. After leaving
  /// the FIFO, it will return to the TAT until the count is exhausted. 
  /// The resubmission to the FIFO has the effect of round-robinning between pending operations.
  ///
  /// A TAT Acc Word has 3 programmable fields:
  ///   stop    : 1 bit, whether or not this is the last entry for the input tag
  ///   AM addr : 10 bits, AM addr to output
  ///   MM addr : 16 bits, MM addr to output
  ///
  /// unlike the PAT, this is a fully-specified MM address. No bits are inferred from the
  /// input tag value, as they are for input spikes
  std::vector<BDWord> PackTATAccWords(const std::vector<unsigned int>& AM_addrs,
                                      const std::vector<unsigned int>& MM_addrs,
                                      const std::vector<unsigned int>& stops) {
    
    unsigned int size = stops.size();
    assert(size == AM_addrs.size());
    assert(size == MM_addrs.size());

    std::vector<BDWord> packed(size);
    for (unsigned int i = 0; i < size; i++) {
      packed[i] = PackWord<TATAccWord>({
        {TATAccWord::AM_ADDRESS, AM_addrs[i]},
        {TATAccWord::MM_ADDRESS, MM_addrs[i]}});
    }
    return packed;
  }
  
  /// Set memory delay line value
  void SetMemoryDelay(unsigned int core_id, bdpars::BDMemId mem_id, unsigned int read_value, unsigned int write_value, bool flush=true);

  /// Program a memory.
  /// BDWords must be constructed as the correct word type for the mem_id
  void SetMem(
      unsigned int core_id,
      bdpars::BDMemId mem_id,
      const std::vector<BDWord> &data,
      unsigned int start_addr);

  /// Default (safe) values for PAT
  /// PAT default is kind of irrelevant, but points to 0, 0
  std::vector<BDWord> GetDefaultPATEntries() const {
    BDWord default_word = PackWord<PATWord>({{PATWord::AM_ADDRESS, 0}, 
                                            {PATWord::MM_ADDRESS_HI, 0}, 
                                            {PATWord::MM_ADDRESS_LO, 0}});
    unsigned int mem_size = bd_pars_->mem_info_.at(bdpars::BDMemId::PAT).size;
    return std::vector<BDWord>(mem_size, default_word);
  }

  /// Default (safe) values for TAT0
  /// TAT emits max tag, max route, stops (stop is critical to avoid infinite loop)
  std::vector<BDWord> GetDefaultTAT0Entries() const {
    BDWord default_word = PackWord<TATTagWord>({{TATTagWord::STOP, 1}, 
                                                {TATTagWord::TAG, (1<<FieldWidth(TATTagWord::TAG)) - 1}, 
                                                {TATTagWord::GLOBAL_ROUTE, (1<<FieldWidth(TATTagWord::GLOBAL_ROUTE)) - 1}});
    unsigned int mem_size = bd_pars_->mem_info_.at(bdpars::BDMemId::TAT0).size;
    return std::vector<BDWord>(mem_size, default_word);
  }

  /// Default (safe) values for TAT1
  /// TAT emits max tag, max route, stops (stop is critical to avoid infinite loop)
  std::vector<BDWord> GetDefaultTAT1Entries() const {
    BDWord default_word = PackWord<TATTagWord>({{TATTagWord::STOP, 1}, 
                                                {TATTagWord::TAG, (1<<FieldWidth(TATTagWord::TAG)) - 1}, 
                                                {TATTagWord::GLOBAL_ROUTE, (1<<FieldWidth(TATTagWord::GLOBAL_ROUTE)) - 1}});
    unsigned int mem_size = bd_pars_->mem_info_.at(bdpars::BDMemId::TAT1).size;
    return std::vector<BDWord>(mem_size, default_word);
  }

  /// Default (safe) values for MM
  /// zero weight squashes everything
  std::vector<BDWord> GetDefaultMMEntries() const {
    unsigned int mem_size = bd_pars_->mem_info_.at(bdpars::BDMemId::MM).size;
    return std::vector<BDWord>(mem_size, 0);
  }

  /// Default (safe) values for AM
  /// stops (critical to avoid looping), emits max tag, max route, threshold is irrelevant
  std::vector<BDWord> GetDefaultAMEntries() const {
    BDWord default_word = PackWord<AMWord>({{AMWord::STOP, 1},
                                          {AMWord::THRESHOLD, 1},
                                          {AMWord::NEXT_ADDRESS, (1<<FieldWidth(AMWord::NEXT_ADDRESS)) - 1}});
    unsigned int mem_size = bd_pars_->mem_info_.at(bdpars::BDMemId::AM).size;
    return std::vector<BDWord>(mem_size, default_word);
  }

  /// Dump the contents of one of the memories.
  /// BDWords must subsequently be unpacked as the correct word type for the mem_id
  std::vector<BDWord> DumpMem(unsigned int core_id, bdpars::BDMemId mem_id);
  /// DumpMem, but specify a specific range of addresses to dump
  /// end is not inclusive, so end=1024 dumps up to element 1023
  std::vector<BDWord> DumpMemRange(unsigned int core_id, bdpars::BDMemId mem_id, unsigned int start, unsigned int end);

  /// Dump copy of traffic pre-FIFO
  void SetPreFIFODumpState(unsigned int core_id, bool dump_en) {
    SetToggleDump(core_id, bdpars::BDHornEP::TOGGLE_PRE_FIFO, dump_en);
  }
  /// Sink traffic flowing into the FIFO
  void SetPreFIFOTrafficState(unsigned int core_id, bool traffic_en) {
    SetToggleTraffic(core_id, bdpars::BDHornEP::TOGGLE_PRE_FIFO, traffic_en);
  }

  /// Dump copy of traffic post-FIFO
  void SetPostFIFODumpState(unsigned int core_id, bool dump_en) {
    SetToggleDump(core_id, bdpars::BDHornEP::TOGGLE_POST_FIFO0, dump_en);
    SetToggleDump(core_id, bdpars::BDHornEP::TOGGLE_POST_FIFO1, dump_en);
  }
  /// Sink traffic flowing out of the FIFO
  void SetPostFIFOTrafficState(unsigned int core_id, bool en) {
    SetToggleTraffic(core_id, bdpars::BDHornEP::TOGGLE_POST_FIFO0, en);
    SetToggleTraffic(core_id, bdpars::BDHornEP::TOGGLE_POST_FIFO1, en);
  }


  /// Get pre-FIFO tags recorded during dump
  std::vector<BDWord> GetPreFIFODump(unsigned int core_id);
  /// Get post-FIFO tags recorded during dump
  std::pair<std::vector<BDWord>, std::vector<BDWord> > GetPostFIFODump(unsigned int core_id);

  /// Get warning count
  std::pair<unsigned int, unsigned int> GetFIFOOverflowCounts(unsigned int core_id);

  //////////////////////////////////////////////////////////////////////////
  // Spike/Tag Streams
  //////////////////////////////////////////////////////////////////////////
  // only RecvSpikes is meant to be used, and only for training
  // otherwise, for tag IO, the FPGA calls should be used

  /// Send a stream of spikes to neurons
  void SendSpikes(
      unsigned int core_id,
      const std::vector<BDWord>& spikes,
      const std::vector<BDTime> times,
      bool flush=true);

  /// Receive a stream of spikes in AER address space
  std::pair<std::vector<BDWord>,
            std::vector<BDTime> > RecvSpikes(unsigned int core_id) {
    // Timeout of 1us
    return RecvFromEP(core_id, bdpars::BDFunnelEP::NRNI, 1);
  }

  std::tuple<std::vector<unsigned int>, std::vector<BDTime>> RecvXYSpikes(unsigned int core_id) {
    auto spike_words = RecvSpikes(core_id);
    auto aer_addresses = spike_words.first;
    auto aer_times = spike_words.second;
    auto num_spikes = aer_addresses.size();

    std::vector<unsigned int> xy_addresses(num_spikes, 0);

    for(unsigned int idx = 0; idx < num_spikes; ++idx){
        auto _addr = aer_addresses[idx];
        if(_addr >=0 && _addr < 4096){
            xy_addresses[idx] = GetSomaXYAddr(aer_addresses[idx]);
        }else {
            cout << "WARNING: Invalid spike address: " << _addr << endl;
        }
    }
    return {xy_addresses, aer_times};
  }

  /// Receive a stream of spikes in XY address space (Y msb, X lsb)
  std::tuple<std::vector<unsigned int>, std::vector<float>> RecvXYSpikesSeconds(unsigned int core_id) {
    auto spike_words = RecvSpikes(core_id);
    auto aer_addresses = spike_words.first;
    auto aer_times = spike_words.second;
    auto num_spikes = aer_addresses.size();

    std::vector<unsigned int> xy_addresses(num_spikes, 0);
    std::vector<float> xy_times(num_spikes, 0);

    for(unsigned int idx = 0; idx < num_spikes; ++idx){
        auto _addr = aer_addresses[idx];
        if(_addr >=0 && _addr < 4096){
            xy_addresses[idx] = GetSomaXYAddr(aer_addresses[idx]);
            xy_times[idx] = static_cast<float>(aer_times[idx]) * 1e-9;
        }else {
            cout << "WARNING: Invalid spike address: " << _addr << endl;
        }
    }
    return {xy_addresses, xy_times};
  }

  /// Receive spikes stream in X-Y flat space as masked boolean array
  std::pair<std::vector<unsigned int>,
            std::vector<float>> RecvXYSpikesMasked(unsigned int core_id);

  /// Send a stream of tags
  void SendTags(
      unsigned int core_id,
      const std::vector<BDWord>& tags,
      const std::vector<BDTime> times={},
      bool flush=true);

  /// Receive a stream of tags
  /// receive from both tag output leaves, the Acc and TAT
  std::pair<std::vector<BDWord>,
            std::vector<BDTime>> RecvTags(unsigned int core_id, unsigned int timeout_us=1000);

  /// receive tags with their fields unpacked, from both tag output leaves, the Acc and TAT
  /// returns {counts, tags, routes, times}
  std::tuple<std::vector<unsigned int>,
             std::vector<unsigned int>,
             std::vector<unsigned int>,
             std::vector<BDTime>> RecvUnpackedTags(unsigned int core_id, unsigned int timeout_us=1000) {
    auto tags_times = RecvTags(core_id, timeout_us);
    std::vector<unsigned int> counts;
    std::vector<unsigned int> tags;
    std::vector<unsigned int> routes;
    for (auto& it : tags_times.first) {
      counts.push_back(GetField(it, TATOutputTag::COUNT));
      tags.push_back(GetField(it, TATOutputTag::TAG));
      routes.push_back(GetField(it, TATOutputTag::GLOBAL_ROUTE));
    }
    return {counts, tags, routes, tags_times.second};
  }

  //////////////////////////////////////////////////////////////////////////
  // FPGA tag IO
  //////////////////////////////////////////////////////////////////////////

  /// Set input rates (in Hz) for Spike Generators.
  /// The SGs are organized as an array, each entry outputting one tag stream
  /// A programming packet specifies the array index of the generator being programmed,
  /// the tag id that it is supposed to output to BD, and the rate that it outputs those tags.
  /// This call allows multiple generators to be set simultaneously.
  /// <time> specifies when the SGs will be programmed (updating the output rates)
  void SetSpikeGeneratorRates(
    unsigned int core_id,
    const std::vector<unsigned int>& gen_idxs, 
    const std::vector<unsigned int>& tags, 
    const std::vector<int>& rates, 
    BDTime time = 0,
    bool flush = true);

  /// Set spike filter increment constant
  void SetSpikeFilterIncrementConst(unsigned int core_id, unsigned int increment, bool flush=true) {
    uint16_t inc_lo = GetField(increment, THREEFPGAREGS::W0);
    uint16_t inc_hi = GetField(increment, THREEFPGAREGS::W1);
    SendToEP(core_id, bd_pars_->DnEPCodeFor(bdpars::FPGARegEP::SF_INCREMENT_CONSTANT0), {inc_lo});
    SendToEP(core_id, bd_pars_->DnEPCodeFor(bdpars::FPGARegEP::SF_INCREMENT_CONSTANT1), {inc_hi});
    if (flush) Flush();
  }

  /// Set spike filter decay constant
  void SetSpikeFilterDecayConst(unsigned int core_id, unsigned int decay, bool flush=true) {
    uint16_t dec_lo = GetField(decay, THREEFPGAREGS::W0);
    uint16_t dec_hi = GetField(decay, THREEFPGAREGS::W1);
    SendToEP(core_id, bd_pars_->DnEPCodeFor(bdpars::FPGARegEP::SF_DECAY_CONSTANT0), {dec_lo});
    SendToEP(core_id, bd_pars_->DnEPCodeFor(bdpars::FPGARegEP::SF_DECAY_CONSTANT1), {dec_hi});
    if (flush) Flush();
  }
  
  /// Set number of spike filters to report
  void SetNumSpikeFilters(unsigned int core_id, unsigned int num, bool flush=true) {
    assert(num <= max_num_SF_);
    SendToEP(core_id, bd_pars_->DnEPCodeFor(bdpars::FPGARegEP::SF_FILTS_USED), {num});
    if (flush) Flush();
  }

  /// Get FPGA SpikeFilter filter ids and states
  /// returns (filter ids, states, times)
  std::tuple<std::vector<unsigned int>,
            std::vector<unsigned int>,
            std::vector<BDTime>> RecvSpikeFilterStates(unsigned int core_id, unsigned int timeout_us) {
    auto words_times = RecvFromEP(core_id, bdpars::FPGAOutputEP::SF_OUTPUT, timeout_us);

    std::vector<unsigned int> filter_ids(words_times.first.size());
    std::vector<unsigned int> filter_states(words_times.first.size());
    for (unsigned int i = 0; i < words_times.first.size(); i++) {
      filter_ids[i]    = GetField(words_times.first[i], FPGASFWORD::FILTIDX);
      filter_states[i] = GetField(words_times.first[i], FPGASFWORD::STATE);
    }
    return {filter_ids, filter_states, words_times.second};
  }

  //////////////////////////////////////////////////////////////////////////
  // Utility
  //////////////////////////////////////////////////////////////////////////

  /// Returns the total number of elements in each output queue
  /// Useful for debugging FPGA issues
  std::vector<std::pair<uint8_t, unsigned int>> GetOutputQueueCounts() {
    std::vector<std::pair<uint8_t, unsigned int>> retvals;
    for (auto& ep_buf : dec_bufs_out_) {
      retvals.push_back({ep_buf.first, ep_buf.second->TotalSize()});
    }
    return retvals;
  }

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


  std::pair<std::vector<BDWord>,
            std::vector<BDTime>>
    RecvFromEPDebug(unsigned int core_id, uint8_t ep_code) {
      return RecvFromEP(core_id, ep_code);
  }
 protected:

  ////////////////////////////////
  // data members
  
  // FPGA state (XXX perhaps should move to its own object)
  static const unsigned int ns_per_clk_  = 10; /// 100 MHz FPGA clock
  static const unsigned int max_num_SF_  = 512; /// number of SF memory entries
  static const unsigned int clks_per_SF_ = 1; /// clock cycles per SF update
  static const unsigned int max_num_SG_  = 256; /// number of SG memory entries
  static const unsigned int clks_per_SG_ = 3; /// clock cycles per SG update
  unsigned int clks_per_unit_         = 1000; /// FPGA default
  unsigned int units_per_HB_          = 100000; /// FPGA default, 1s HB (really long!)
  BDTime ns_per_unit_                 = ns_per_clk_ * clks_per_unit_; /// FPGA default
  BDTime highest_ns_sent_             = 0;

  // basis of experiment time, set when ResetFPGAClock is called
  std::chrono::high_resolution_clock::time_point base_time_ = std::chrono::high_resolution_clock::now(); 


  /// array mapping SG generator idx -> enabled/disabled
  std::vector<std::array<bool, max_num_SG_>> SG_en_;
  void InitSGEn(unsigned int core_id) {
    for (auto& it : SG_en_.at(core_id)) {
      it = false;
    } 
  }
  /// send current SG_en_ values
  void SendSGEns(unsigned int core_id, BDTime time);

  /// FPGA time units per microsecond
  inline uint64_t NsToUnits(BDTime   ns)    { return ns / ns_per_unit_; }
  inline BDTime   UnitsToNs(uint64_t units) { return ns_per_unit_ * units; }

  /// FPGA SG_en_ max bit assigned helper
  inline int GetHighestSGEn(unsigned int core_id) const {
    int highest = -1;
    for (unsigned int i = 0; i < SG_en_.at(core_id).size(); i++) {
      if (SG_en_.at(core_id).at(i)) highest = i;
    }
    return highest;
  }

  /// Number of upstream "push"s sent.
  /// There's a hardware bug where every upstream word has to be "pushed" out of the 
  /// synchronizer queue by subsequent BD traffic. The number of elements waiting to be
  /// pushed out at any given time is 2.
  /// For calls that are dependent on a particular number of elements to be returned,
  /// we issue some additional upstream words to get the last two words we need.
  /// a "push" is a PAT dump (read)
  unsigned int num_pushs_pending_ = 0;

  /// Issues two push words (PAT dumps) to force out the 2 trapped words
  void IssuePushWords();

  /// parameters describing BD hardware
  const bdpars::BDPars *bd_pars_;
  /// best-of-driver's-knowledge state of bd hardware
  std::vector<BDState> bd_state_;

  /// parameters describing Opal Kelly hardware
  OKPars ok_pars_;

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
  std::unordered_map<uint8_t, VectorDeserializer<DecOutput> *> up_ep_deserializers_;

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

  std::unordered_map<unsigned int, std::vector<bool>, EnumClassHash> last_traffic_state_;

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

  // helpers for DumpMem
  void DumpMemSend(unsigned int core_id, bdpars::BDMemId mem_id, unsigned int start_addr, unsigned int end_addr);
  std::vector<BDWord> DumpMemRecv(unsigned int core_id, bdpars::BDMemId mem_id, unsigned int dump_first_n, unsigned int wait_for_us);

  ////////////////////////////////
  // register programming helpers

  void SetBDRegister(unsigned int core_id, bdpars::BDHornEP reg_id, BDWord word, bool flush=true);
  void SetToggle(unsigned int core_id, bdpars::BDHornEP toggle_id, bool traffic_enable, bool dump_enable, bool flush=true);
  bool SetToggleTraffic(unsigned int core_id, bdpars::BDHornEP reg_id, bool en, bool flush=true);
  bool SetToggleDump(unsigned int core_id, bdpars::BDHornEP reg_id, bool en, bool flush=true);


};

}  // bddriver namespace
}  // pystorm namespace

#endif
