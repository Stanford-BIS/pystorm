//manages sending packets from this board to the next

module interboard_output(
	input [10:0] fifo_data, //data we're reading from the fifo (q port)
	input read_input, //read signal from the board we're transmitting to
	input rdempty, //if read is empty
	input input_clk, //clock from the board we're transmitting to
	input reset,
	output reg valid, //valid signal, sent to the board we're transmitting to
	output reg [10:0] send_data, //data sent to next board
	output reg rdreq, //read request for dual clock fifo
	output wire rdclk //clock for fifo
	);

//send clock signal to fifo
assign rdclk = input_clk;

//send words to output
assign send_data = fifo_data;

//signals latched on rdclk
always @(posedge rdclk) begin
	if (reset == 1) begin
		rdreq <= 1'b0;
		valid <= 1'b0;
	end
	else begin
		//if asking to read, read (no issue with reading from empty fifo)
		rdreq <= read_input;

		//valid signal is high if read input is high and fifo is not empty
		valid <= (~rdempty) & read_input;
	end
end

endmodule 
