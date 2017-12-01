//manages receving packets sent to this board by interboard_output.sv

module interboard_input(
	input transmit_clk, //the clock speed we're transmitting data at
	input valid, //valid signal from the board we're recieving data from
	input [10:0] recieve_data, //data recieved from the other board
	input [7:0] wrusedw, //fifo words remaining
	input reset,
	output reg [10:0] data, //data sent to the fifo
	output wire wrclk, //write clock for fifo
	output reg wrreq, //write request for fifo
	output reg read, //read request for next board
	output wire out_clk //output clock for other boards
	);

//fifo triggers
parameter fifo_stop_trigger = 8'd255; //one less than max
parameter fifo_start_trigger = 8'd254; //two less than max

//send clock to fifo
assign wrclk = transmit_clk;
assign out_clk = transmit_clk;

//signals latched on transmit_clk
always @(posedge transmit_clk) begin
	if (reset == 1) begin
		data <= 11'b0;
		wrreq <= 1'b0;
		read <= 1'b0;
	end
	else begin
		//send recieved data to fifo
		data <= recieve_data;

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