`define SIMULATION
`include "../src/core/BDEncoder.sv"
`include "ChannelSrcSink.sv"

`timescale 1ns / 1ps

module BDFunnelEncoder_tb;

Channel #(21) BD_data_out();
UnencodedBDWordChannel words_in();

// clock
logic clk;
parameter Tclk = 10;
always #(Tclk/2) clk = ~clk;
initial clk = 0;

// reset
logic reset;
initial begin
  reset <= 0;
  @(posedge clk) reset <= 1;
  @(posedge clk) reset <= 0;
end

Channel #(26) words_in_packed();
assign {words_in.leaf_code, words_in.payload} = words_in_packed.d;
assign words_in.v = words_in_packed.v;
assign words_in_packed.a = words_in.a;
RandomChannelSrc #(.N(26)) words_in_src(words_in_packed, clk, reset);

ChannelSink out_sink(BD_data_out, clk, reset);

BDFunnelEncoder dut(.*);

endmodule

module BDFunnelSerializer_tb;

UnencodedBDWordChannel words_in();
UnencodedBDWordChannel words_out();

// clock
logic clk;
parameter Tclk = 10;
always #(Tclk/2) clk = ~clk;
initial clk = 0;

// reset
logic reset;
initial begin
  reset <= 0;
  @(posedge clk) reset <= 1;
  @(posedge clk) reset <= 0;
end

Channel #(26) words_in_packed();
assign {words_in.leaf_code, words_in.payload} = words_in_packed.d;
assign words_in.v = words_in_packed.v;
assign words_in_packed.a = words_in.a;
RandomChannelSrc #(.N(26)) words_in_src(words_in_packed, clk, reset);

Channel #(26) words_out_packed();
assign words_out_packed.d = {words_out.leaf_code, words_out.payload};
assign words_out_packed.v = words_out.v;
assign words_out.a = words_out_packed.a;
ChannelSink out_sink(words_out_packed, clk, reset);

BDFunnelSerializer dut(.*);

endmodule
