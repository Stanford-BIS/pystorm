`include "../lib/Interfaces.svh" 
`include "../lib/Channel.svh"
`include "TimeUnitPulser.sv"

// Responsible for keeping the wall clock, in time_units
// Also responsible for generating control signals for the downstream traffic.
// If PC_time_elapsed is current or behind, traffic is running,
// If PC_time_elapsed is ahead, then traffic blocks.
// Generates signals for when to send heartbeats in upstream traffic
module TimeMgr #(
  parameter Nunit = 16, 
  parameter Ntime = 48) (

  // used by other time-dependent units
  output logic unit_pulse, // pulses every time_unit, for 1 clk cycle
  output logic send_HB_up_pulse, // pulses every send_HB_up_every * time_unit, for 1 clk cycle
  output logic [Ntime-1:0] time_elapsed, // current wall time

  // downstream controls
  output logic stall_dn, // waiting for wall clock to catch up to PC clock

  // inputs from PCParser
  TimeMgrConf conf,

  input clk, reset);

// 2^Nunit = time unit max val, in clock cycles
// 2^Ntime = max time value, in epochs

// generate a pulse every time unit
TimeUnitPulser #(Nunit) unit_pulser(.clks_per_unit(conf.unit_len), .*);

// increment units_elapsed based on unit_pulse
always_ff @(posedge clk, posedge reset)
  if (reset == 1)
    time_elapsed <= 1;
  else 
    if (conf.reset_time == 1)
      time_elapsed <= 1; 
    else if (unit_pulse == 1)
      time_elapsed <= time_elapsed + 1;

// generate upstream heartbeat
logic [Ntime-1:0] time_units_since_HB;
always_ff @(posedge clk, posedge reset)
  if (reset == 1) begin
    time_units_since_HB <= 1;
    send_HB_up_pulse <= 0;
  end
  else 
    if (unit_pulse == 1)
      if (time_units_since_HB >= conf.send_HB_up_every) begin
        time_units_since_HB <= 1;
        send_HB_up_pulse <= 1;
      end
      else begin
        time_units_since_HB <= time_units_since_HB + 1;
        send_HB_up_pulse <= 0;
      end
    else begin
      time_units_since_HB <= time_units_since_HB;
      send_HB_up_pulse <= 0;
    end

// generate stall signal (it's a clock cycle behind, no biggie)
logic next_stall_dn;
always_ff @(posedge clk, posedge reset)
  if (reset == 1)
    stall_dn <= 0;
  else
    stall_dn <= next_stall_dn;

always_comb
  if (conf.PC_time_elapsed > time_elapsed)
    next_stall_dn = 1;
  else
    next_stall_dn = 0;

endmodule
