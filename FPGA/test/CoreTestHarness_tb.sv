`include "../src/CoreTestHarness.sv"
`include "ChannelSrcSink.sv"

module CoreTestHarness #(
  parameter NPCcode = 8,
  parameter PCtoBDcode = {NPCcode{1'b1}},
  parameter BDtoPCcode = {NPCcode{1'b1}})
(
  Channel PC_in,
  Channel PC_out,
  input clk, reset);

// instantiates a core without a connection to BD
// adds a split to PC_in to BD_in
// adds a merge from BD_out to PC_out
// this way the PC can inject simulated traffic from BD and can
// receive all traffic that would go to BD
// uses "111..1" codes for both BD_out -> PC streams and PC -> BD_in streams

localparam NPCdata = 24;
localparam NPCout = NPCcode + NPCdata;
localparam NPCin = NPCcode + NPCdata;

localparam NBDdata_in = 34;
localparam NBDdata_out = 21;

localparam NPCword = NPCcode + NPCdata;

// PC-side
Channel #(NPCword) core_PC_in();
Channel #(NPCword) core_PC_out();

// BD-side
Channel #(NBDdata_out) core_BD_out();
Channel #(NBDdata_in) core_BD_in();

// misc
logic pReset;
logic sReset;

logic adc0;
logic adc1;

////////////////////////////////////////
// PC -> BD_in

// split traffic from PC, code sent into BD is "111..1"
Channel #(NPCword) PC_to_BD_in();
ChannelSplit #(
  NPCword, 
  {{NPCcode{1'b1}}, {NPCdata{1'b0}}}, 
  {PCtoBDcode, {NPCdata{1'b0}}})
input_split(
  PC_to_BD_in,
  core_PC_in,
  PC_in);

// deserialize PC_in -> BD_in payload
Channel #(NPCdata) PC_to_BD_in_payload();
assign PC_to_BD_in_payload.d = PC_to_BD_in.d[NPCdata-1:0];
assign PC_to_BD_in_payload.v = PC_to_BD_in.v;
assign PC_to_BD_in.a = PC_to_BD_in_payload.a;

Deserializer #(NPCdata, NBDdata_in) BD_in_deser(core_BD_in, PC_to_BD_in_payload, clk, reset);

////////////////////////////////////////
// BD_out -> PC

// merge BD_out with PC_out, code sent upstream for BD traffic is "111..1"
Channel #(NPCword) BD_out_to_PC();
assign BD_out_to_PC.v = core_BD_out.v;
assign BD_out_to_PC.d = {BDtoPCcode, {3{1'b0}}, core_BD_out.d};
assign core_BD_out.a = BD_out_to_PC.a;

ChannelMerge output_merge(PC_out, core_PC_out, BD_out_to_PC, clk, reset);

// instantiate core
Core core(.PC_out(core_PC_out), .PC_in(core_PC_in), .BD_out(core_BD_out), .BD_in(core_BD_in), .*);

endmodule
