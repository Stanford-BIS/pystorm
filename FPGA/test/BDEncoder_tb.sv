`include "../src/BDEncoder.sv"
`include "ChannelSrcSink.sv"

module BDEncoder_tb;

Channel #(21) BD_out();
UnencodedBDWordChannel enc_in();

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

Channel #(26) enc_in_packed();
assign {enc_in.leaf_code, enc_in.payload} = enc_in_packed.d;
assign enc_in.v = enc_in_packed.v;
assign enc_in_packed.a = enc_in.a;
RandomChannelSrc #(.N(26)) enc_in_src(enc_in_packed, clk, reset);

ChannelSink out_sink(BD_out, clk, reset);

BDEncoder dut(.*);

endmodule
