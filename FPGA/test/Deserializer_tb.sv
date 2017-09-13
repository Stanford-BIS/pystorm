`include "../src/Deserializer.sv"
`include "ChannelSrcSink.sv"

module Deserializer_tb;

parameter Nin = 4;
parameter Nout = 13;

Channel #(Nin) in(); // Nin wide
Channel #(Nout) out(); // Nout wide

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

// source
RandomChannelSrc #(.N(Nin)) src(in, clk, reset);

// sink
ChannelSink sink(out, clk, reset);

Deserializer #(Nin, Nout) dut(.*);

endmodule
