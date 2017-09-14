`define SIMULATION
`include "../src/Serializer.sv"
`include "ChannelSrcSink.sv"

module Serializer_tb;

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

Channel #(36) in();
RandomChannelSrc #(.N(36)) in_src(in, clk, reset);

Channel #(16) out();
ChannelSink out_sink(out, clk, reset);

Serializer #(.Nin(36), .Nout(16)) dut(.*);

endmodule
