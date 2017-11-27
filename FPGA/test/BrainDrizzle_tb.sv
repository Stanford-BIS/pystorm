`define SIMULATION
`include "../src/Router/BrainDrizzle.sv"

`timescale 10ps/1ps
module BrainDrizzle_tb();

 reg clk;
 reg reset; 
 reg top_in_clk;
 reg bot_in_clk;
 reg BD_in_clk;
 reg top_valid_in;
 reg bot_valid_in;
 reg BD_valid_in;
 reg top_ready_in;
 reg BD_ready_in;
 reg bot_ready_in;
 reg [10:0] top_in;
 reg [10:0] bot_in;
 reg [10:0] BD_in;
 wire [10:0] top_out;
 wire [10:0] bot_out;
 wire [10:0] BD_out;
 wire top_valid_out;
 wire bot_valid_out;
 wire BD_valid_out;
 wire top_ready_out;
 wire bot_ready_out;
 wire BD_ready_out;
 wire top_out_clk;
 wire BD_out_clk;
 wire bot_out_clk;
 
 reg [3:0] top_packet_count=0;
 reg [3:0] BD_packet_count=0;
 reg [3:0] bot_packet_count=0;
 reg [10:0] top_data_in;
 reg [10:0] save_top_data_in;
 reg [10:0] BD_data_in;
 reg [10:0] bot_data_in;
 wire [10:0] top_data_out;
 wire [10:0] BD_data_out;
 wire [10:0] bot_data_out;
 wire top_rdreq;
 wire BD_rdreq;
 wire bot_rdreq;
 wire top_wr;
 wire BD_wr;
 wire bot_wr;
 reg [7:0] top_rdusedw;
 reg [7:0] BD_rdusedw;
 reg [7:0] bot_rdusedw;
 reg [7:0] top_wrusedw;
 reg [7:0] BD_wrusedw;
 reg [7:0] bot_wrusedw;
 reg top_empty;
 
 reg [10:0] top_test_input;
 reg top_in_full;
 reg top_wr_in;
 reg top_readout_test;
 reg bot_readout_test;
 reg BD_readout_test;


BrainDrizzle DUT (
 .clk				(clk),
 .reset			(reset),
 .top_in_clk	(top_in_clk),
 .bot_in_clk	(bot_in_clk),
 .BD_in_clk		(BD_in_clk),
 .top_valid_in	(top_valid_in),
 .bot_valid_in	(bot_valid_in),
 .BD_valid_in	(BD_valid_in),
 .top_ready_in	(top_ready_in),
 .BD_ready_in	(BD_ready_in),
 .bot_ready_in	(bot_ready_in),
 .top_in			(top_in),
 .bot_in			(bot_in),
 .BD_in			(BD_in),
 .top_out		(top_out),
 .bot_out		(bot_out),
 .BD_out			(BD_out),
 .top_valid_out(top_valid_out),
 .bot_valid_out(bot_valid_out),
 .BD_valid_out	(BD_valid_out),
 .top_ready_out(top_ready_out),
 .BD_ready_out	(BD_ready_out),
 .bot_ready_out(bot_ready_out),
 .top_out_clk	(top_out_clk),
 .BD_out_clk	(BD_out_clk),
 .bot_out_clk	(bot_out_clk)
);

//module interboard_output(
//	input [10:0] fifo_data, //data we're reading from the fifo (q port)
//	input read_input, //read signal from the board we're transmitting to
//	input rdempty, //if read is empty
//	input input_clk, //faster clock from this board
//	input reset,
//	output reg valid, //valid signal, sent to the board we're transmitting to
//	output reg [10:0] send_data, //data sent to next board
//	output reg rdreq //read request for dual clock fifo
//	);
interboard_output DUT_top_in (
	.fifo_data	(top_data_in),
	.read_input	(top_ready_out),
	.empty		(top_empty),
	.input_clk	(top_in_clk),
	.reset		(reset),
	.valid		(top_valid_in),
	.send_data	(top_in),
	.rdreq		(top_rdreq)
);
//module routerDCFIFO (
//	data,
//	rdclk,
//	rdreq,
//	wrclk,
//	wrreq,
//	q,
//	rdempty,
//	rdusedw,
//	wrfull,
//	wrusedw);

routerDCFIFO top_input (
	.data		(top_test_input),
	.rdclk	(top_in_clk),
	.rdreq	(top_rdreq),
	.wrclk	(clk),
	.wrreq	(top_wr_in),
	.q			(top_data_in),
	.rdempty	(top_empty),
	.rdusedw	(top_rdusedw),
	.wrfull	(top_in_full),
	.wrusedw	()
	);
	


//module interboard_input(
//	input transmit_clk, //clock from board we're receiving input from
//	input valid, //valid signal from the board we're recieving data from
//	input [10:0] receive_data, //data recieved from the other board
//	input [7:0] wrusedw, //fifo words remaining
//	input reset,
//	output reg [10:0] data, //data sent to the fifo
//	output reg wrreq, //write request for fifo
//	output reg read //read request for next board
//	);
interboard_input DUT_bot_out (
	.transmit_clk(bot_out_clk),
	.valid(bot_valid_out),
	.receive_data(bot_out),
	.wrusedw(bot_wrusedw),
	.reset(reset),
	.data(bot_data_out),
	.wrreq(bot_wr),
	.read(bot_ready_in)
);

interboard_input DUT_BD_out (
	.transmit_clk(BD_out_clk),
	.valid(BD_valid_out),
	.receive_data(BD_out),
	.wrusedw(BD_wrusedw),
	.reset(reset),
	.data(BD_data_out),
	.wrreq(BD_wr),
	.read(BD_ready_in)
);

interboard_input DUT_top_out (
	.transmit_clk(top_out_clk),
	.valid(top_valid_out),
	.receive_data(top_out),
	.wrusedw(top_wrusedw),
	.reset(reset),
	.data(top_data_out),
	.wrreq(top_wr),
	.read(top_ready_in)
);

routerDCFIFO bot_output (
	.data		(bot_data_out),
	.rdclk	(bot_out_clk),
	.rdreq	(bot_readout_test),
	.wrclk	(bot_out_clk),
	.wrreq	(bot_wr),
	.q			(bot_output_test),
	.rdempty	(),
	.rdusedw	(),
	.wrfull	(),
	.wrusedw	(bot_wrusedw)
);
	routerDCFIFO BD_output (
	.data		(BD_data_out),
	.rdclk	(BD_out_clk),
	.rdreq	(BD_readout_test),
	.wrclk	(BD_out_clk),
	.wrreq	(BD_wr),
	.q			(BD_output_test),
	.rdempty	(),
	.rdusedw	(),
	.wrfull	(),
	.wrusedw	(BD_wrusedw)
);
	

initial
begin
	#0
	clk=0;
	top_in_clk=0;
	bot_in_clk=0;
	BD_in_clk=0;
	bot_readout_test=0;
	BD_readout_test=0;
	reset=1;
	top_test_input=11'b01111111100;
	
	#110
	reset=0;
	
	#50000
	BD_readout_test=1;
	bot_readout_test=1;

end

always 
	#50 clk = !clk;
	
always begin
	#50
	top_in_clk=!top_in_clk;
	bot_in_clk=!bot_in_clk;
	BD_in_clk=!BD_in_clk;
end

always @ (posedge top_in_clk) begin
	if(~top_in_full) begin
		top_wr_in=1;
		top_test_input[9:0] = top_test_input[9:0]+1;
		top_packet_count = top_packet_count+1;
		if(top_packet_count==6) begin
			top_packet_count=0;
			top_test_input[10]=1;
		end
		else
			top_test_input[10]=0;
	end
	else
		top_wr_in=0;
end

endmodule

