
/////////////////////////////////////////////////////
//Sends a reset signal using the tail bit pins.
//
//A reset is triggered by the tail bit being high for 4 clock cycles.
/////////////////////////////////////////////////////

module tail_bit_reset#(parameter N = 3)(
	input below_clk, //clock from the board below us
	input our_clk, //this board's clock
	input [10:0] bot_in, //the value of the tail bit pin
	input bot_valid_in, //valid signal
	output logic reset, //reset signal for this board
	output reg[10:0] top_out_reset, //the tail bit to send to the next board
	output reg valid_reset
	);

always @(posedge below_clk) begin
	reset <= (&bot_in) & (~bot_valid_in);
	if((&bot_in) & (~bot_valid_in)) begin
		top_out_reset <= bot_in;
		valid_reset <= 0;
	end
	else begin
		top_out_reset <= 11'b0;
		valid_reset <= 1;
	end
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