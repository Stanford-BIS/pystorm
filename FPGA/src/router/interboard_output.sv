//manages sending packets from this board to the next

module interboard_output(
	input [10:0] fifo_data, //data we're reading from the fifo (q port)
	input read_input, //read signal from the board we're transmitting to
	input empty, //if read is empty
	input input_clk, //faster clock from this board
	input reset,
	output reg valid, //valid signal, sent to the board we're transmitting to
	output reg [10:0] send_data, //data sent to next board
	output reg rdreq //read request for dual clock fifo
	);

reg valid_ff=0;

always @(posedge input_clk)
begin: valid_flipflop
	if(~empty)
		valid <= valid_ff;
	else
		valid <= 0;
end



//signals latched on rdclk
always @(posedge input_clk) begin
	if (reset == 1) begin
		rdreq <= 1'b0;
		valid_ff <= 1'b0;
		send_data <=0;
	end
	else begin
		send_data <= fifo_data;
		if(~empty & read_input)begin
			//
			rdreq <= 1;
			//valid signal is high if read input is high
			valid_ff <= 1;
		end
		else begin
			rdreq <= 0;
			valid_ff <= 0;
		end
	end
end

endmodule 
