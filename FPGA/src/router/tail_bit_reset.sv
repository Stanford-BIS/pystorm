
/////////////////////////////////////////////////////
//Sends a reset signal using the tail bit pins.
//
//A reset is triggered by the tail bit being high for 4 clock cycles.
/////////////////////////////////////////////////////

module tail_bit_reset(
	input below_clk, //clock from the board below us
	input our_clk, //this board's clock
	input tail_bit, //the value of the tail bit pin
	output reg reset, //reset signal for this board
	output reg next_tail_bit //the tail bit to send to the next board
	);

reg int1, int2, int3; //intermediate input signals
assign mid = int1 & int2 & int3; //reset is high if all of these are high

reg sc1, sc2, sc3; //sync (totally not brute force, this is very nuanced)

reg out1, out2, out3; //intermediate output signals


//input ffs
always @(posedge below_clk) begin
	int1 <= tail_bit;
	int2 <= int1;
	int3 <= int2;
end

//output ffs
always @(posedge our_clk) begin
	sc1 <= mid;
	sc2 <= sc1;
	sc3 <= sc2;
	out1 <= sc3;
	out2 <= out1;
	out3 <= out2;
	next_tail_bit <= out1 | out2 | out3; //next tail bit is held high
	reset <= next_tail_bit; //hold reset high too
end

endmodule // tail_bit_reset