`include "Interfaces.svh" 
`include "Channel.svh"

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

// generate stall signal
always_comb
  if (conf.PC_time_elapsed > time_elapsed)
    stall_dn = 1;
  else
    stall_dn = 0;

endmodule

// TESTBENCH
module TimeMgr_tb;

parameter Nunit = 16;
parameter Ntime = 48;

logic unit_pulse;
logic send_HB_up_pulse;
logic [Ntime-1:0] time_elapsed;

logic stall_dn;

TimeMgrConf #(Nunit, Ntime) conf();

logic clk;
logic reset;

parameter Tclk = 10;

always #(Tclk/2) clk = ~clk;

parameter ClksPerUnit = 8; // 4 clks is one unit
parameter UnitsPerHB = 4;

// PC clock comes in at random times
DatalessChannel send_PC_time();
DatalessChannelSrc #(.ClkDelaysMin(ClksPerUnit/2), .ClkDelaysMax(ClksPerUnit/2*3)) random_delay_maker(send_PC_time, clk, reset);
assign send_PC_time.a = 1; // always ack, we just want the delay

always @(posedge send_PC_time.v)
  conf.PC_time_elapsed <= conf.PC_time_elapsed + 1;

initial begin
  clk = 0;
  conf.unit_len = ClksPerUnit; 
  conf.send_HB_up_every = UnitsPerHB;
  conf.PC_time_elapsed = 0;

  reset = 1;
  #(Tclk) reset = 0;
  #(Tclk) conf.reset_time = 1;
  #(Tclk) conf.reset_time = 0;
end

TimeMgr #(.Ntime(Ntime), .Nunit(Nunit)) dut(.*);

endmodule
