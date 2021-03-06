`ifndef INTERFACES_SVH
`define INTERFACES_SVH

`include "Channel.svh"

// register bundles
interface SpikeFilterConf #(parameter Nfilts = 10, parameter Nstate = 27);
  logic [Nfilts-1:0] filts_used;         // max filter idx in use
  logic [Nstate-1:0] increment_constant; // increment state by this on spike
  logic [Nstate-1:0] decay_constant;     // multiply state by this each time unit
endinterface

interface SpikeGeneratorConf #(parameter Ngens = 8);
  logic [Ngens-1:0] gens_used; // max gen idx in use
  logic [(2**Ngens)-1:0] gens_en;   // enable signal for each generator
endinterface

interface TimeMgrConf #(parameter Nunit = 16, parameter Ntime = 48);
  logic reset_time;                        // reset FPGA wall clock
  logic [Nunit-1:0] unit_len;              // set FPGA wall clock time unit
  logic [Ntime-1:0] PC_time_elapsed;       // PC's downstream heartbeat
  logic [Ntime-1:0] send_HB_up_every;  // send upstream heartbeat every <x> time units
endinterface

interface TagSplitConf;
  logic report_tags;
endinterface

interface BDIOConf;
  logic pReset;
  logic sReset;
endinterface

// channels
interface TagCtChannel #(parameter Ntag = 11, parameter Nct = 9);
  logic [Ntag-1:0] tag;
  logic [Nct-1:0] ct;
  logic v;
  logic a;
endinterface

interface GlobalTagCtChannel #(parameter Nglobal = 12, parameter Ntag = 11, parameter Nct = 9);
  logic [Nglobal-1:0] global_tag;
  logic [Ntag-1:0] tag;
  logic [Nct-1:0] ct;
  logic v;
  logic a;
endinterface

interface SpikeFilterOutputChannel #(parameter Nfilts = 10, parameter Nstate = 27);
  logic [Nfilts-1:0] filt_idx;
  logic [Nstate-1:0] filt_state;
  logic v;
  logic a;
endinterface

interface SpikeGeneratorProgChannel #(parameter Ngens = 8, parameter Nperiod = 16, parameter Ntag = 11);
  logic [Ngens-1:0] gen_idx;
  logic [Nperiod-1:0] period;
  logic [Nperiod-1:0] ticks;
  logic [Ntag-1:0] tag;
  logic sign;
  logic v;
  logic a;
endinterface

interface UnencodedBDWordChannel;
  logic [5:0] leaf_code; // there are 34 leaves
  logic [23:0] payload; 
  logic v;
  logic a;
endinterface

interface DecodedBDWordChannel;
  logic [3:0] leaf_code; // there are 13 leaves (and one INVALID code)
  logic [37:0] payload; // the longest data chunk
  logic v;
  logic a;
endinterface

interface SerializedPCWordChannel;
  logic [7:0] code;
  logic [23:0] payload;
  logic v;
  logic a;
endinterface

interface SerializedPCWordChannelwithRoute;
  logic [7:0] code;
  logic [23:0] payload;
  logic [9:0] route;
  logic v;
  logic a;
endinterface

`endif
