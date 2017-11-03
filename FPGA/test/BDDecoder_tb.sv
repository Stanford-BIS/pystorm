`define SIMULATION
`include "../src/core/BDDecoder.sv"
`include "ChannelSrcSink.sv"

module BDHornDecoder_tb;

Channel #(34) BD_in();
DecodedBDWordChannel dec_out();

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

RandomChannelSrc #(.N(34)) BD_src(BD_in, clk, reset);

Channel #(38) dec_out_packed();
assign dec_out_packed.v = dec_out.v;
assign dec_out_packed.d = {dec_out.leaf_code, dec_out.payload};
assign dec_out.a = dec_out_packed.a;
ChannelSink ser_sink(dec_out_packed, clk, reset);

BDHornDecoder dut(.*);

endmodule

module BDHornSerializer_tb;

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

DecodedBDWordChannel words_in();
DecodedBDWordChannel words_out();

localparam N = 38;

Channel #(N) words_in_packed();
assign {words_in.leaf_code, words_in.payload} = words_in_packed.d;
assign words_in.v = words_in_packed.v;
assign words_in_packed.a = words_in.a;
RandomChannelSrc #(.N(N)) in_src(words_in_packed, clk, reset);

Channel #(N) words_out_packed();
assign words_out_packed.d = {words_out.leaf_code, words_out.payload};
assign words_out_packed.v = words_out.v;
assign words_out.a = words_out_packed.a;
ChannelSink out_sink(words_out_packed, clk, reset);

BDHornDeserializer dut(.*);

endmodule
