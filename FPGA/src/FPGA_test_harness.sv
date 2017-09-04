`include "Channel.svh"

module FPGA_test_harness (
  Channel PC_in,
  Channel PC_out,
  input clk, reset)

  // instantiates a core without a connection to BD
  // adds a split to PC_in to BD_in
  // adds a merge from BD_out to PC_out
  // this way the PC can inject simulated traffic from BD and can
  // receive all traffic that would go to BD

  parameter NPCcode = 8;
  parameter NPCdata = 24;
  parameter NPCout = NPCcode + NPCdata;
  parameter NPCin = NPCcode + NPCdata;

  parameter NBDdata_in = 34;
  parameter NBDdata_out = 21;

  parameter NPCword = NPCcode + NPCdata;

  // PC-side
  Channel #(NPCword) core_PC_in();
  Channel #(NPCword) core_PC_out();

  // BD-side
  Channel #(NBDdata_out) core_BD_out();
  Channel #(NBDdata_in) core_BD_in();

  // split traffic from PC
  Channel #(NPCword) PC_to_BD_in();
  ChannelSplit #(
    NPCword, 
    {{NPCcode{1'b1'}}, {NPCdata{1'b0}}}, 
    {{NPCcode{1'b1'}}, {NPCdata{1'b0}}})
  input_split(
    core_PC_in,
    PC_to_BD_in,
    PC_in);

  // deserialize PC_in -> BD_in payload
  Channel #(NPCdata) PC_to_BD_in_payload();
  assign PC_to_BD_in_payload.d = PC_to_BD_in.d[NPCdata-1:0];
  assign PC_to_BD_in_payload.v = PC_to_BD_in.v;
  assign PC_to_BD_in.a = PC_to_BD_in_payload.a;

  Deserializer #(NPCdata, NBDdata_in) BD_in_deser(core_BD_in, PC_to_BD_in_payload);

  // merge BD_out with PC_out
  Channel #(NPCword) BD_out_to_PC;
  assign BD_out_to_PC_out.v = core_BD_out.v;
  assign BD_out_to_PC_out.d = {{NPCcode{core_BD_out.v}}, {3{1'b1}}, core_BD_out.d};
  assign core_BD_out.a = BD_out_to_PC_out.a;

  ChannelMerge (PC_out, core_PC_out, BD_out_to_PC_out);

  // instantiate core
  Core core(core_PC_in, core_PC_out, core_BD_out, core_BD_in);

endmodule
