`ifndef TIMEMGR_SVH
`define TIMEMGR_SVH

// Responsible for keeping the epoch and intra-epoch wall clocks.
// Also responsible for generating control signals for the downstream traffic
// can either be stalling traffic during a wait event, or squashing
// wait events if the PC is running behind.
// Generates signals for when to send heartbeats in upstream traffic
// CAVEAT: you are going to lose 1/2 a time_unit per wait event in this
// implementation, SW should take this into account
module TimeMgr #(
  parameter Nunit = 16, 
  parameter Nepoch = 10, 
  parameter Ntime  = 32,
  parameter SquashThr = 1) (

  // upstream controls
  Channel send_heartbeat_up, // should send heartbeat to PC with epoch ct

  // downstream controls
  output logic stall_dn,          // inside delay event or waiting for heartbeat, stall stream to BD
  output logic squash_delay_dn,   // PC running behind more than one epoch, squash delays so it catches up

  // inputs from PCParser
  TimeMgrCtrlInputs ctrl_in,

  input clk, 
  input reset);

`endif
