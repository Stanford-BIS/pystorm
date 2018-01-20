`define SIMULATION
`include "../src/router/BrainDrizzle.sv"

`timescale 10ps/1ps
module BrainDrizzle_2_nodes_tb();

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
 reg [10:0] bot_2_in;
 reg [10:0] BD_2_in;
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
 reg [3:0] BD_2_packet_count=0;
 reg [3:0] bot_2_packet_count=0;
 reg [10:0] top_data_in;
 reg [10:0] BD_data_in;
 reg [10:0] BD_2_data_in;
 reg [10:0] bot_2_data_in;
 wire top_rdreq;
 wire BD_rdreq;
 wire BD_2_rdreq;
 wire bot_2_rdreq;
 reg top_empty;
 reg BD_empty;
 reg BD_2_empty;
 reg bot_2_empty;
 

 reg BD_2_valid_in;
 reg bot_2_valid_in;
 


BrainDrizzle DUT_1 (
 .clk				(clk),
 .reset			(reset),
 .top_in_clk	(top_in_clk),
 .bot_in_clk	(bot_in_clk),
 .BD_in_clk		(BD_in_clk),
 .top_valid_in	(top_valid_in),
 .bot_valid_in	(bot_valid_in),
 .BD_valid_in	(BD_valid_in),
 .top_ready_in	(1),
 .BD_ready_in	(1),
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
 .bot_out_clk	(bot_out_clk),
 .sent_something_to_top	(),
 .sent_something_to_BD	()
);

BrainDrizzle DUT_2 (
 .clk				(clk),
 .reset			(reset),
 .top_in_clk	(bot_out_clk),
 .bot_in_clk	(bot_in_clk),
 .BD_in_clk		(BD_in_clk),
 .top_valid_in	(bot_valid_out),
 .bot_valid_in	(bot_2_valid_in),
 .BD_valid_in	(BD_2_valid_in),
 .top_ready_in	(bot_ready_out),
 .BD_ready_in	(1),
 .bot_ready_in	(1),
 .top_in			(bot_out),
 .bot_in			(bot_2_in),
 .BD_in			(BD_2_in),
 .top_out		(bot_in),
 .bot_out		(),
 .BD_out			(),
 .top_valid_out(bot_valid_in),
 .bot_valid_out(),
 .BD_valid_out	(),
 .top_ready_out(bot_ready_in),
 .BD_ready_out	(BD_2_ready_out),
 .bot_ready_out(bot_2_ready_out),
 .top_out_clk	(bot_in_clk),
 .BD_out_clk	(),
 .bot_out_clk	()
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
interboard_output DUT_1_top_in (
	.fifo_data	(top_data_in),
	.read_input	(top_ready_out),
	.empty		(top_empty),
	.input_clk	(top_in_clk),
	.reset		(reset),
	.valid		(top_valid_in),
	.send_data	(top_in),
	.rdreq		(top_rdreq)
);
interboard_output DUT_1_BD_in (
	.fifo_data	(BD_data_in),
	.read_input	(BD_ready_out),
	.empty		(BD_empty),
	.input_clk	(BD_in_clk),
	.reset		(reset),
	.valid		(BD_valid_in),
	.send_data	(BD_in),
	.rdreq		(BD_rdreq)
);

interboard_output DUT_2_bot_in (
	.fifo_data	(bot_2_data_in),
	.read_input	(bot_2_ready_out),
	.empty		(bot_2_empty),
	.input_clk	(bot_in_clk),
	.reset		(reset),
	.valid		(bot_2_valid_in),
	.send_data	(bot_2_in),
	.rdreq		(bot_2_rdreq)
);
interboard_output DUT_2_BD_in (
	.fifo_data	(BD_2_data_in),
	.read_input	(BD_2_ready_out),
	.empty		(BD_2_empty),
	.input_clk	(BD_in_clk),
	.reset		(reset),
	.valid		(BD_2_valid_in),
	.send_data	(BD_2_in),
	.rdreq		(BD_2_rdreq)
);

	

initial
begin
	#0
	clk=0;
	top_in_clk=0;
	bot_in_clk=0;
	BD_in_clk=0;
	reset=1;
	top_data_in=11'd0;
	BD_data_in=11'b01111111111;
	BD_2_data_in=11'b01111111111;
	bot_2_data_in=11'd0;
	top_empty=1;
	BD_empty=0;
	BD_2_empty=1;
	bot_2_empty=1;
	
	#110
	reset=0;
	
	#800
	BD_empty=1;
	
	#2500
	BD_2_empty=0;
	bot_2_empty=0;
	

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
	if(top_rdreq) begin
		top_data_in = {top_data_in[10], top_data_in[9:0]+1};
		top_packet_count=top_packet_count+1;
	end
	if(top_packet_count >=4) begin
		top_data_in[10]=1;
		top_packet_count=0;
	end
		
end
always @ (posedge BD_in_clk) begin
	if(BD_rdreq) begin
		BD_data_in = {BD_data_in[10], BD_data_in[9:0]+1};
		BD_packet_count=BD_packet_count+1;
	end
	if(BD_packet_count >=4) begin
		BD_data_in[10]=1;
		BD_packet_count=0;
	end
		
end
always @ (posedge bot_in_clk) begin
	if(bot_2_rdreq) begin
		bot_2_data_in = {bot_2_data_in[10], bot_2_data_in[9:0]+1};
		bot_2_packet_count=bot_2_packet_count+1;
	end
	if(bot_2_packet_count >=4) begin
		bot_2_data_in[10]=1;
		bot_2_packet_count=0;
	end
		
end
always @ (posedge BD_in_clk) begin
	if(BD_2_rdreq) begin
		BD_2_data_in = {BD_2_data_in[10], BD_2_data_in[9:0]+1};
		BD_2_packet_count=BD_2_packet_count+1;
	end
	if(BD_2_packet_count >=4) begin
		BD_2_data_in[10]=1;
		BD_2_packet_count=0;
	end
		
end

endmodule

