`ifndef INTERFACES_SVH
`define INTERFACES_SVH

`include "Channel.svh"

// Collects the various interfaces shared between major components.

// PCParser -> TimeMgr
//
// reset_time: sets the time_unit counter to 0. Typically used at the start of an experiment
// unit_len: set the time unit duration, in clock cycles
//
// PC_time_elapsed: time PC intended to send subsequent stream elements, in time units

interface TimeMgrCtrlInputs #(parameter Nunit = 16, parameter Ntime = 40);
  logic reset_time;
  logic [Nunit-1:0] unit_len;

  logic [Ntime-1:0] PC_time_elapsed;
endinterface

interface TagCtChannel #(parameter Ntag = 11, parameter Nct = 10);
  logic [Ntag-1:0] tag;
  logic [Nct-1:0] ct;
  logic r;
  logic v;
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
  logic r;
  logic v;
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
