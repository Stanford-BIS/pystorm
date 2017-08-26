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

interface TimeMgrConf #(parameter Nunit = 16, parameter Ntime_lo = 20, parameter Ntime_hi = 20);
  logic reset_time;                        // reset FPGA wall clock
  logic [Nunit-1:0] unit_len;              // set FPGA wall clock time unit
  logic [Ntime_lo-1:0] PC_time_elapsed_lo; // PC's downstream heartbeat, LSBs
  logic [Ntime_hi-1:0] PC_time_elapsed_hi; // PC's downstream heartbeat, MSBs
endinterface


// channels
interface TagCtChannel #(parameter Ntag = 11, parameter Nct = 10);
  logic [Ntag-1:0] tag;
  logic [Nct-1:0] ct;
  logic v;
  logic a;
endinterface

interface PassiveTagCtChannel #(parameter Ntag = 11, parameter Nct = 10);
  logic [Ntag-1:0] tag;
  logic [Nct-1:0] ct;
  logic r;
  logic v;
endinterface

interface FilterOutputChannel #(parameter Nstate = 27, parameter Nfilts = 10, parameter Nct = 10);
  logic [Nstate-1:0] filt_state;
  logic [Nfilts-1:0] filt_idx;
  logic v;
  logic a;
endinterface

interface ProgramSpikeGeneratorChannel #(parameter Ngens = 8, parameter Nperiod = 16, parameter Ntag = 11);
  logic [Ngens-1:0] gen_idx;
  logic [Nperiod-1:0] period;
  logic [Nperiod-1:0] ticks;
  logic [Ntag-1:0] tag;
  logic v;
  logic a;
endinterface

`endif
