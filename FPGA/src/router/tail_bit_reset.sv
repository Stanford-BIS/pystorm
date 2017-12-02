
/////////////////////////////////////////////////////
//Sends a reset signal using the tail bit pins.
//
//A reset is triggered by the tail bit being high for 4 clock cycles.
/////////////////////////////////////////////////////

module tail_bit_reset(
	input below_clk, //clock from the board below us
	input our_clk, //this board's clock
	input top_clk, //the clock from above this board
	input tail_bit, //the value of the tail bit pin
	output reg below_reset, //reset on below clock domain
	output reg reset, //reset signal for this board
	output reg top_reset, //reset for top clock domain
	output reg next_tail_bit //the tail bit to send to the next board
	);

reg int1, int2, int3; //intermediate input signals
assign mid = int1 & int2 & int3; //reset is high if all of these are high

reg sc1, sc2, sc3; //sync (totally not brute force, this is very nuanced)
reg tsc1, tsc2, tsc3; //sync (totally not brute force, this is very nuanced)

reg bout1, bout2, bout3; //intermediate output signals
reg out1, out2, out3; //intermediate output signals
reg tout1, tout2, tout3; //intermediate output signals


//input ffs
always @(posedge below_clk) begin
	int1 <= tail_bit;
	int2 <= int1;
	int3 <= int2;

	//bot out ffss
	bout1 <= mid;
	bout2 <= bout1;
	bout3 <= bout2;
	below_reset <= bout1 | bout2 | bout3;
end

//output ffs for our out
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

//output ffs for top out
always @(posedge top_clk) begin
	tsc1 <= mid;
	tsc2 <= tsc1;
	tsc3 <= tsc2;
	tout1 <= tsc3;
	tout2 <= tout1;
	tout3 <= tout2;
	top_reset <= tout1 | tout2 | tout3;
end 

endmodule // tail_bit_reset