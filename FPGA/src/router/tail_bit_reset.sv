
/////////////////////////////////////////////////////
//Sends a reset signal using the tail bit pins.
//
//A reset is triggered by the tail bit being high for 4 clock cycles.
/////////////////////////////////////////////////////

module tail_bit_reset#(parameter N = 3)(
	input below_clk, //clock from the board below us
	input our_clk, //this board's clock
	input tail_bit, //the value of the tail bit pin
	output reset, //reset signal for this board
	output next_tail_bit //the tail bit to send to the next board
	);

reg [N-1:0] in; //intermediate input signals
//wire mid = &in; //reset is high if all of these are high

assign reset = &in;
assign next_tail_bit = reset;

reg [N-1:0] sc; //sync (totally not brute force, this is very nuanced)

reg [N-1:0] out; //intermediate output signals


//input ffs
always @(posedge our_clk) begin
	in[0] <= tail_bit;
	for(int i=1; i<N; i++) in[i] <= in[i-1];
end

//output ffs
//always @(posedge our_clk) begin
//	sc[0] <= mid;
//	for(int i=1; i<N; i++) sc[i] <= sc[i-1];
//
//	out[0] <= sc[N-1];
//	for(int i=1; i<N; i++) out[i] <= out[i-1];
//
//	next_tail_bit <= |out; //next tail bit is held high
//	reset <= next_tail_bit; //hold reset high too
//end

endmodule // tail_bit_reset