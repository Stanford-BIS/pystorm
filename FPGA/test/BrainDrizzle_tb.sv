`define SIMULATION
`include "../src/Router/BrainDrizzle.sv"

`timescale 10ps/1ps
module BrainDrizzle_tb();

 reg clk; 
 reg top_in_clk;
 reg bot_in_clk;
 reg BD_in_clk;
 reg valid_top;
 reg valid_bot;
 reg valid_BD;
 reg [10:0] top_in;
 reg [10:0] bot_in;
 reg [10:0] BD_in;
//Will be removed after Zach is finished with inter-board handshaking
 reg top_out_read;
 reg BD_out_read;
 reg bot_out_read;
 reg top_in_wr;
 reg BD_in_wr;
 reg bot_in_wr;
 wire top_in_full;
 wire BD_in_full;
 wire bot_in_full;
 wire top_out_empty;
 wire BD_out_empty;
 wire bot_out_empty;
////////////////////////////////
 wire [10:0] top_out;
 wire [10:0] bot_out;
 wire [10:0] BD_out;
 wire ready_top;
 wire ready_bot;
 wire ready_BD;
 wire top_out_clk;
 wire BD_out_clk;
 wire bot_out_clk;
 
 reg [3:0] top_packet_count=0;
 reg [3:0] BD_packet_count=0;
 reg [3:0] bot_packet_count=0;
 reg wr_en;

BrainDrizzle DUT (
 .clk			(clk), 
 .top_in_clk	(top_in_clk),
 .bot_in_clk	(bot_in_clk),
 .BD_in_clk		(BD_in_clk),
 .valid_top		(),
 .valid_bot		(),
 .valid_BD		(),
 .top_in		(top_in),
 .bot_in		(bot_in),
 .BD_in			(BD_in),
//Will be removed after Zach is finished with inter-board handshaking
 .top_out_read	(top_out_read),
 .BD_out_read	(BD_out_read),
 .bot_out_read	(bot_out_read),
 .top_in_wr		(top_in_wr),
 .BD_in_wr		(BD_in_wr),
 .bot_in_wr		(bot_in_wr),
 .top_in_full	(top_in_full),
 .BD_in_full	(BD_in_full),
 .bot_in_full	(bot_in_full),
 .top_out_empty	(top_out_empty),
 .BD_out_empty	(BD_out_empty),
 .bot_out_empty	(bot_out_empty),
////////////////////////////////
 .top_out		(top_out),
 .bot_out		(bot_out),
 .BD_out		(BD_out),
 .ready_top		(),
 .ready_bot		(),
 .ready_BD		(),
 .top_out_clk	(),
 .BD_out_clk	(),
 .bot_out_clk	()
);

initial
begin
	#0
	clk=0;
	top_in_clk=0;
	bot_in_clk=0;
	BD_in_clk=0;
	top_in=11'b01111111110;
	bot_in=0;
	BD_in=11'b01111110000;
	top_out_read=0;
	BD_out_read=0;
	bot_out_read=0;
	top_in_wr=0;
	BD_in_wr=0;
	bot_in_wr=0;
	wr_en=1;
	
	// #150
	// top_in[10]=1;
	// BD_in[10]=1;
	
	// #50
	// top_in[10]=0;
	// BD_in[10]=0;
	
	// #500
	// top_in[10]=1;
	// BD_in[10]=1;
	
	// #50
	// top_in[10]=0;
	// BD_in[10]=0;
	
	#175
	wr_en=0;
	
	#675
	wr_en=1;
	
	#1000
	top_out_read=1;
	BD_out_read=1;
	bot_out_read=1;
	

	
end

always 
	#50 clk = !clk;
	
always begin
	#25
	top_in_clk=!top_in_clk;
	bot_in_clk=!bot_in_clk;
	BD_in_clk=!BD_in_clk;
end

always @ (negedge top_in_clk) begin
	if(top_in_full==0 && wr_en) begin
		top_in[9:0] = top_in[9:0]+1;
		top_in_wr=1;
		top_packet_count=top_packet_count+1;
		if(top_packet_count==6) begin
			top_packet_count=0;
			top_in[10]=1;
		end
		else
			top_in[10]=0;
			
	end
	else
		top_in_wr=0;
	if(BD_in_full==0 && wr_en) begin
		BD_in[9:0] = BD_in[9:0]-1;
		BD_in_wr=1;
		BD_packet_count=BD_packet_count+1;
		if(BD_packet_count==6) begin
			BD_packet_count=0;
			BD_in[10]=1;
		end
		else
			BD_in[10]=0;
	end
	else
		BD_in_wr=0;
end



endmodule

