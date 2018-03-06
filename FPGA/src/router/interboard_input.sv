//manages receving packets sent to this board by interboard_output.sv

module interboard_input(
	input transmit_clk, //clock from board we're receiving input from
	input valid, //valid signal from the board we're recieving data from
	input [10:0] receive_data, //data recieved from the other board
	input [7:0] wrusedw, //fifo words remaining
	input reset,
	output reg [10:0] data, //data sent to the fifo
	output reg wrreq, //write request for fifo
	output reg read //read request for next board
	);

//fifo triggers
parameter fifo_stop_trigger = 8'd240; //one less than max
parameter fifo_start_trigger = 8'd239; //two less than max


//signals latched on transmit_clk
always @(posedge transmit_clk or posedge reset) begin
	if (reset == 1) begin
		data <= 11'b0;
		wrreq <= 1'b0;
		read <= 1'b0;
	end
	else begin
		//send recieved data to fifo
		data <= receive_data;

		//if valid data, write to fifo
		wrreq <= valid;

		//if we're at or above stop trigger, pull read low
		if (wrusedw >= fifo_stop_trigger) begin
			read <= 1'b0;
		end
		//if we're at or below stop trigger, pull read high
		else if (wrusedw <= fifo_start_trigger) begin
			read <= 1'b1;
		end
		//else, keep read at what it was
		else begin
			read <= read;
		end
	end
end

endmodule