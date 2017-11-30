`include "../src/router/tail_bit_reset.sv"

module tail_bit_reset_tb;

// clock
logic clk;
parameter Tclk = 10;
always #(Tclk/2) clk = ~clk;
initial clk = 0;

reg bottom_tail_bit; //our intput; hold high to reset the stack

reg to1, to2, to3; //tail bit outputs
reg r1, r2, r3; //resets

tail_bit_reset b1(clk, clk, bottom_tail_bit, r1, to1);
tail_bit_reset b2(clk, clk, to1, r2, to2);
tail_bit_reset b3(clk, clk, to2, r3, to3);

initial begin
	repeat(4) @(posedge clk) bottom_tail_bit <= 1; //test normal reset
	repeat(15) @(posedge clk) bottom_tail_bit <= 0;
	repeat(3) @(posedge clk) bottom_tail_bit <= 1; //test too-short reset
	repeat(15) @(posedge clk) bottom_tail_bit <= 0;
	repeat(6) @(posedge clk) bottom_tail_bit <= 1; //test long
	@(posedge clk) bottom_tail_bit <= 0;
end

endmodule // tail_bit_reset_tb