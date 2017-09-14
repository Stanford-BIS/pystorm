`define SIMULATION
`include "../src/BDDecoder.sv"
`include "ChannelSrcSink.sv"

module BDDecoder_tb;

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

Channel #(32) dec_out_packed();
assign dec_out_packed.v = dec_out.v;
assign dec_out_packed.d = {dec_out.leaf_code, dec_out.payload};
assign dec_out.a = dec_out_packed.a;
ChannelSink ser_sink(dec_out_packed, clk, reset);

BDDecoder dut(.*);

endmodule
