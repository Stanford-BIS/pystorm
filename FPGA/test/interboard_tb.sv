`define SIMULATION

`include "../src/router/interboard_input.sv"
`include "../src/router/interboard_output.sv"
`timescale 10ps/1ps

module interboard_tb();
reg clk;
reg reset;

//send_board_fifo
reg send_board_fifo_rdreq;
reg send_board_fifo_rdclk;
reg send_board_fifo_rdempty;
wire [10:0] send_board_fifo_out = send_board_fifo_rdreq ? 11'd17 : 11'd0;

//interboard signals from sending board
reg [10:0] interboard_data;
reg [10:0] interboard_data_del;
reg valid;
reg valid_del;

//interboard signals from receiving board
reg read;
reg read_del;
reg interboard_clk;
reg interboard_clk_del;

//recieve_board_fifo
reg [10:0] receive_board_fifo_in;
reg receive_board_fifo_wrreq;
reg receive_board_fifo_wrclk;
reg [7:0] receive_board_fifo_wrusedw;

interboard_input inputDUT(
	.transmit_clk(clk), //the clock speed we're transmitting data at
	.valid(valid_del), //valid signal from the board we're recieving data from
	.recieve_data(interboard_data_del), //data recieved from the other board
	.wrusedw(receive_board_fifo_wrusedw), //fifo words remaining
	.reset(reset),
	.data(receive_board_fifo_in), //data sent to the fifo
	.wrclk(receive_board_fifo_wrclk), //write clock for fifo
	.wrreq(receive_board_fifo_wrreq), //write request for fifo
	.read(read), //read request for next board
	.out_clk(interboard_clk) //out clock for other board
	);

interboard_output outputDUT(
	.fifo_data(send_board_fifo_out), //data we're reading from the fifo (q port)
	.read_input(read_del), //read signal from the board we're transmitting to
	.rdempty(send_board_fifo_rdempty), //if read is empty
	.input_clk(interboard_clk_del), //clock from the board we're transmitting to
	.reset(reset),
	.valid(valid), //valid signal, sent to the board we're transmitting to
	.send_data(interboard_data), //data sent to next board
	.rdreq(send_board_fifo_rdreq), //read request for dual clock fifo
	.rdclk(send_board_fifo_rdclk) //clock for fifo
	);

initial begin
	#0
	clk = 0;
	reset = 1;
	send_board_fifo_rdempty = 0;
	receive_board_fifo_wrusedw = 0;

	#50
	reset = 0;
	//should begin sending data here

	#80
	receive_board_fifo_wrusedw = 8'd255;
	//we've filled up, should stop transmitting

	#80
	receive_board_fifo_wrusedw = 8'd254;
	//have a spot, should start transmitting
end

//delay signals
always begin
	#2
	interboard_data_del = interboard_data;
	interboard_clk_del = interboard_clk;
	valid_del = valid;
	read_del = read;
end

always begin
	#10 clk = ~clk;
end

endmodule