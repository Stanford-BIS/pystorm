`include "../quartus/BDIOPLL.v"
`include "../quartus/BDIOPLL/BDIOPLL_0002.v"

module BDClkGen (
  output wire BD_in_clk_int,
  output wire BD_in_clk_ext,
  output wire BD_out_clk_int,
  output wire BD_out_clk_ext,
  output wire pll_locked,
  input clk, reset);

wire skewed_clk; // unskewed 20MHz clock to use in handshaker
wire base_clk;   // skewed 10MHz clock to use for BD IO

BDIOPLL BC_pll(
  .refclk(clk),   // input clock(), 100 MHz
  .rst(reset),
  .outclk_0(fast_clk),
  .outclk_1(slow_clk),
  .outclk_2(slow_skewed_clk),
  .locked(pll_locked));

assign BD_in_clk_ext = slow_skewed_clk;
assign BD_out_clk_ext = slow_skewed_clk;

assign BD_in_clk_int = fast_clk;
assign BD_out_clk_int = slow_clk;

endmodule
