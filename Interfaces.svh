`ifndef INTERFACES_SVH
`define INTERFACES_SVH

`include "Channel.svh"

// Collects the various interfaces shared between major components.

// PCParser -> TimeMgr
//
// reset_time: sets the epoch to 0. Typically used at the signals the start of an experiment
//   doesn't need to be a DatalessChannel
// unit_len: set the time unit duration, in clock cycles
// epoch_len: set the epoch duration, in time units
// PC_epochs_elapsed: PC's current epoch
//
// do_wait: stream of wait events. TimeMgr needs to stall on repeated waits,
//  so it's a channel

interface TimeMgrCtrlInputs #(parameter Nunit = 16, parameter Nepoch = 10, parameter Ntime = 32);
  logic reset_time;
  logic [Nunit-1:0] unit_len;
  logic [Nepoch-1:0] epoch_len;
  logic [Ntime-1:0] PC_epochs_elapsed;

  Channel #(Nepoch) do_wait();
endinterface

`endif
