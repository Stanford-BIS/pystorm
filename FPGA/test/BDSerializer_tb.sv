`include "../src/BDSerializer.sv"
`include "ChannelSrcSink.sv"

module BDSerializer_tb;

DecodedBDWordChannel dec_in();
SerializedPCWordChannel ser_out();

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

Channel #(36) dec_in_packed();
assign dec_in.v = dec_in_packed.v;
assign {dec_in.leaf_code, dec_in.payload} = dec_in_packed.d;
assign dec_in_packed.a = dec_in.a;
RandomChannelSrc #(.N(36)) dec_in_src(dec_in_packed, clk, reset);

Channel #(16) ser_out_packed();
assign ser_out_packed.v = ser_out.v;
assign ser_out_packed.d = {ser_out.code, ser_out.payload};
assign ser_out.a = ser_out_packed.a;
ChannelSink ser_out_sink(ser_out_packed, clk, reset);

BDSerializer dut(.*);

endmodule
