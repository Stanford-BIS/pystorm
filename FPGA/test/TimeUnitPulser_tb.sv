`include "../src/TimeUnitPulser.sv"
`include "ChannelSrcSink.sv"

module TimeUnitPulser_tb;

parameter N = 16;

logic unit_pulse;
logic[N-1:0] clks_per_unit = 'D4;
logic clk;
logic reset;

parameter Tclk = 10;

always #(Tclk/2) clk = ~clk;

initial begin
  clk = 0;
  #(Tclk) reset = 1;
  #(Tclk) reset = 0;
end

TimeUnitPulser dut(.*);

endmodule

