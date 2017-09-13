module BDClkGen (
  output wire BD_clk_int_in,
  output wire BD_clk_ext_in,
  output wire BD_clk_int_out,
  output wire BD_clk_ext_out,
  input clk, reset);

wire locked; // unused

wire skewed_clk; // unskewed 10MHz clock to use in handshaker
wire base_clk;   // skewed 10MHz clock to use for BD IO

BDIOPLL BC_pll(
  .refclk(clk),   // input clock(), 100 MHz
  .rst(reset),
  .outclk_0(base_clk)  
  .outclk_1(skewed_clk)  
  .locked(locked),
  );

assign BD_clk_ext_in = skewed_clk;
assign BD_clk_ext_out = skewed_clk;

assign BD_clk_int_in = base_clk;
assign BD_clk_int_out = base_clk;

endmodule
