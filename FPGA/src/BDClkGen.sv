// for quartus, we add external IP to the project
`ifdef SIMULATION
  `include "../quartus/BDIOPLL.v"
  `include "../quartus/BDIOPLL/BDIOPLL_0002.v"
`endif

module BDClkGen (
  output wire BD_in_clk_int,
  output wire BD_in_clk_ext,
  output wire BD_out_clk_int,
  output wire BD_out_clk_ext,
  input clk, reset);

wire skewed_clk; // unskewed 10MHz clock to use in handshaker
wire base_clk;   // skewed 10MHz clock to use for BD IO

BDIOPLL BC_pll(
  .refclk(clk),   // input clock(), 100 MHz
  .rst(reset),
  .outclk_0(base_clk),
  .outclk_1(skewed_clk));

assign BD_in_clk_ext = skewed_clk;
assign BD_out_clk_ext = skewed_clk;

assign BD_in_clk_int = base_clk;
assign BD_out_clk_int = base_clk;

endmodule
