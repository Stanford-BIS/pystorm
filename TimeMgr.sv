`include "Interfaces.svh" 
`include "Channel.svh"

`include "TimeUnitPulser.sv" // for now, only I include this

// Responsible for keeping the wall clock, in time_units
// Also responsible for generating control signals for the downstream traffic.
// If PC_time_elapsed is current or behind, traffic is running,
// If PC_time_elapsed is ahead, then traffic blocks.
// Generates signals for when to send heartbeats in upstream traffic
module TimeMgr #(
  parameter Nunit = 16, 
  parameter Ntime_hi = 20,
  parameter Ntime_lo = 20) (

  // used by other time-dependent units
  output logic unit_pulse,                           // pulses every time_unit, for 1 clk cycle
  output logic [Ntime_hi+Ntime_lo-1:0] time_elapsed, // current wall time

  // downstream controls
  output logic stall_dn, // waiting for wall clock to catch up to PC clock

  // inputs from PCParser
  TimeMgrCtrlInputs ctrl_in,

  input clk, 
  input reset);

parameter Ntime = Ntime_hi + Ntime_lo;

// 2^Nunit = time unit max val, in clock cycles
// 2^Nepoch = epoch length, in time units
// 2^Ntime = max time value, in epochs

// generate a pulse every time unit
TimeUnitPulser #(Nunit) unit_pulser(.clks_per_unit(ctrl_in.unit_len), .*);

// increment units_elapsed based on unit_pulse
always_ff @(posedge clk, posedge reset)
  if (reset == 1)
    time_elapsed <= 1;
  else 
    if (ctrl_in.reset_time == 1)
      time_elapsed <= 1; 
    else if (unit_pulse == 1)
      time_elapsed <= time_elapsed + 1;

always_comb
  if (ctrl_in.PC_time_elapsed > time_elapsed)
    stall_dn = 1;
  else
    stall_dn = 0;

endmodule

// TESTBENCH
module TimeMgr_tb;

parameter Nunit = 16;
parameter Ntime_hi = 20;
parameter Ntime_lo = 20;
parameter Ntime = Ntime_hi + Ntime_lo;

logic unit_pulse;
logic [Ntime_hi+Ntime_lo-1:0] time_elapsed;

logic stall_dn;

TimeMgrCtrlInputs #(Nunit, Ntime) ctrl_in();

logic clk;
logic reset;

parameter Tclk = 10;

always #(Tclk/2) clk = ~clk;

parameter ClksPerUnit = 8; // 4 clks is one unit

// PC clock comes in at random times
DatalessChannel send_PC_time();
DatalessChannelSrc #(.ClkDelaysMin(ClksPerUnit/2), .ClkDelaysMax(ClksPerUnit/2*3)) random_delay_maker(send_PC_time, clk, reset);
assign send_PC_time.a = 1; // always ack, we just want the delay

always @(posedge send_PC_time.v)
  ctrl_in.PC_time_elapsed <= ctrl_in.PC_time_elapsed + 1;

initial begin
  clk = 0;
  ctrl_in.unit_len = ClksPerUnit; 
  ctrl_in.PC_time_elapsed = 0;

  reset = 1;
  #(Tclk) reset = 0;
  #(Tclk) ctrl_in.reset_time = 1;
  #(Tclk) ctrl_in.reset_time = 0;
end

TimeMgr dut(.*);

endmodule
