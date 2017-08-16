`include "Interfaces.svh" 
`include "Channel.svh"

`include "TimeUnitPulser.sv" // for now, only I include this

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

// 2^Nunit = time unit max val, in clock cycles
// 2^Nepoch = epoch length, in time units
// 2^Ntime = max time value, in epochs

logic [Ntime-1:0] epochs_elapsed; // FPGA's internal wall clock, epochs elapsed since reset
logic [Nepoch-1:0] units_elapsed; // number of time units elapsed in this epoch
logic [Nepoch-1:0] wait_countdown; // used to control stall_dn signal

// generate a pulse every time unit
logic unit_pulse; // pulse for every time unit's passage
TimeUnitPulser #(Nunit) unit_pulser(.clks_per_unit(ctrl_in.unit_len), .*);

// increment units_elapsed based on unit_pulse
// increment epochs_elapsed when units_elapsed overflows
always_ff @(posedge clk, posedge reset)
  if (reset == 1) begin
    epochs_elapsed <= 0;
    units_elapsed <= 0;
  end
  else 
    if (ctrl_in.reset_time == 1) begin
      epochs_elapsed <= 0;
      units_elapsed <= 1; 
    end
    else if (unit_pulse == 1)
      if (units_elapsed >= ctrl_in.epoch_len) begin
        epochs_elapsed <= epochs_elapsed + 1;
        units_elapsed <= 1;
      end
      else 
        units_elapsed <= units_elapsed + 1;

// generate send_heartbeat_up handshake using basic FSM
logic send_heartbeat_cond;
always_comb
  if ((units_elapsed >= ctrl_in.epoch_len) & (unit_pulse == 1))
    send_heartbeat_cond = 1;
  else
    send_heartbeat_cond = 0;

ChannelSender up_sender_fsm(send_heartbeat_up.v, send_heartbeat_up.a, send_heartbeat_cond, clk, reset);

always_comb
  if (send_heartbeat_up.v == 1)
    send_heartbeat_up.d = epochs_elapsed;
  else
    send_heartbeat_up.d = 'X;


// FSM states for downstream traffic
enum {RUN, STALL, SQUASH} dn_state;

// downstream outputs
always_comb
  unique case (dn_state)
    RUN: begin
      stall_dn = 0;
      squash_delay_dn = 0;
    end
    STALL: begin
      stall_dn = 1;
      squash_delay_dn = 0;
    end
    SQUASH: begin
      stall_dn = 0;
      squash_delay_dn = 1;
    end
  endcase


// note: need to go back to RUN for a cycle between stalls,
// the do_wait.a logic currently depends on that
// means we can't handshake do_wait at max throughput,
// but waits are so long it doesn't really matter
always_ff @(posedge clk, posedge reset)
  unique case (dn_state)
    RUN: 
      // RUN -> SQUASH if epoch_dn is running behind
      if (epochs_elapsed > ctrl_in.PC_epochs_elapsed + SquashThr) 
        dn_state <= SQUASH;
      // RUN -> STALL if new wait event
      else if (ctrl_in.do_wait.v)
        dn_state <= STALL;
      else
        dn_state <= RUN;
    STALL:
      // STALL -> RUN if wait_countdown = 0
      if (wait_countdown == 0)
        dn_state <= RUN;
      else
        dn_state <= STALL;
    SQUASH:
      // SQUASH -> RUN when PC catches up
      if (epochs_elapsed <= ctrl_in.PC_epochs_elapsed + SquashThr) 
        dn_state <= RUN;
      else
        dn_state <= SQUASH;
  endcase

// handshake do_wait.a
// if (do_wait.v & RUN), then we will transition to STALL, ack
//   there's one unit of slack, we acked before the STALL was over,
//   but subsequent waits will block while under STALL
// if (do_wait.v & SQUASH), then we're ignoring wait events, ack (without
//   doing anything with the payload)
// if (do_wait.v & STALL), do not ack
assign ctrl_in.do_wait.a = ctrl_in.do_wait.v & ((dn_state == RUN) | (dn_state == SQUASH));

// load up wait_countdown and decrement it every time_unit until it reaches zero,
// entering SQUASH zeroes the countdown
always_ff @(posedge clk, posedge reset)
  if (reset == 1)
    wait_countdown <= 0;
  else
    case (dn_state)
      RUN: begin
        assert (wait_countdown == 0);
        if (ctrl_in.do_wait.v)
          wait_countdown <= ctrl_in.do_wait.d;
      end
      STALL:
        if (unit_pulse == 1)
          wait_countdown <= wait_countdown - 1;
      SQUASH:
        wait_countdown <= 0;
    endcase

endmodule

module TimeMgr_tb;

parameter Nunit = 16;
parameter Nepoch = 10;
parameter Ntime  = 32;
parameter SquashThr = 2;

Channel #(Nepoch) send_heartbeat_up();
logic stall_dn;
logic squash_delay_dn;

TimeMgrCtrlInputs #(Nunit, Nepoch, Ntime) ctrl_in();
logic clk;
logic reset;

parameter Tclk = 10;

always #(Tclk/2) clk = ~clk;

parameter ClksPerUnit = 4; // 4 clks is one unit
parameter UnitsPerEpoch = 4; // 16 clks is one epoch
parameter MaxDelayUnits = 10; // random source for wait events (0-10 time units delay)
RandomChannelSrc #(.N(Nepoch), .Max(4), .Min(4), .ClkDelaysMin(0), .ClkDelaysMax(MaxDelayUnits*ClksPerUnit)) wait_src(ctrl_in.do_wait, clk, reset);

ChannelSink #(.ClkDelaysMax(ClksPerUnit*UnitsPerEpoch/2)) hb_sink(send_heartbeat_up, clk, reset);

// can simulate falling behind
logic block_PC_update;
always @(dut.epochs_elapsed)
  if (block_PC_update == 0)
    ctrl_in.PC_epochs_elapsed <= dut.epochs_elapsed;

initial begin
  clk = 0;
  ctrl_in.unit_len = ClksPerUnit; 
  ctrl_in.epoch_len = UnitsPerEpoch;
  ctrl_in.PC_epochs_elapsed = 0;
  block_PC_update = 1;

  reset = 1;
  #(Tclk) reset = 0;
  #(Tclk) ctrl_in.reset_time = 1;
  #(Tclk) ctrl_in.reset_time = 0;

  #(Tclk * ClksPerUnit * UnitsPerEpoch * 3) block_PC_update = 0;

end

TimeMgr dut(.*);

endmodule
