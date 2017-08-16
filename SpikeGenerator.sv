`include "Channel.svh"

module SpikeGenerator #(parameter N = 16) (
  DatalessChannel spike_out, // notification to send spike, arbited against other SpikeGenerators
  input en, // enable
  input [N-1:0] period, // in time units
  input time_unit_pulse, // from TimeMgr
  input clk,
  input reset);

logic [N-1:0] count;

// counter keeps running independently of state update
// precise timing is preserved as long as SENDING isn't
// held up for more than one time unit
always_ff @(posedge clk, posedge reset)
  if (reset == 1 || en == 0)
    count <= 1;
  else
    if (time_unit_pulse == 1)
      if (count >= period) 
        count <= 1;
      else
        count <= count + 1;

logic send_condition;
always_comb
  if ((time_unit_pulse == 1) & (count == period))
    send_condition = 1;
  else
    send_condition = 0;

ChannelSender sender_fsm(spike_out.v, spike_out.a, send_condition, clk, reset);

endmodule


module SpikeGenerator_tb;

parameter N = 16;

DatalessChannel spike_out(); // notification to send spike; arbited against other SpikeGenerators
logic en; // enable
logic [N-1:0] period; // in time units
logic time_unit_pulse; // from TimeMgr
logic clk;
logic reset;

parameter Tclk = 10;

always #(Tclk/2) clk = ~clk;

parameter Tpulse = 4; // clks

// time_unit_pulse
always begin
  #(Tclk) time_unit_pulse <= 0;
  #(Tclk * Tpulse) time_unit_pulse <= 1;
end

initial begin
  clk <= 0;
  reset <= 1;
  period <= 2;
  en <= 1;
  #(Tclk) reset <= 0;

  #(Tclk * 20) period <= 5;

  #(Tclk * 40) period <= 10;

  #(Tclk * 100) en <= 0;
end

SpikeGenerator dut(.*);

DatalessChannelSink #(.ClkDelaysMax(2)) spike_sink(spike_out, clk, reset);

endmodule

