`define SIMULATION
`include "../src/router/stack_core.sv"

`include "BDSrcSink.sv"

`timescale 1ns/1ps
module stack_core_tb();

reg osc; //signal for oscillator

//input from other boards
reg top_in_clk;
reg bot_in_clk;
reg top_valid_in;
reg bot_valid_in;
reg top_ready_in;
reg bot_ready_in;
reg [10:0] top_in;
reg [10:0] bot_in;
reg [10:0] top_out;
reg [10:0] bot_out;
reg top_valid_out;
reg bot_valid_out;
reg top_ready_out;
reg bot_ready_out;
reg top_out_clk;
reg bot_out_clk;

//BD-ward signals
reg BD_out_clk_ifc;
reg BD_out_ready;
reg BD_out_valid;
reg [20:0] BD_out_data;
reg BD_in_clk_ifc;
reg BD_in_ready;
reg _BD_in_valid;
reg [33:0] BD_in_data;
reg pReset;
reg sReset;
reg adc0;
reg adc1;

reg reset = 0; //for srcs

//basic routing tests for router -> BD
initial
begin
	#0
	osc=0;
	top_in_clk=0;
	bot_in_clk=0;
	BD_in_clk_ifc=0;
	bot_in=11'b11111111111; //reset
	bot_valid_in = 0;
	bot_ready_in = 1;
	#600
	bot_in=11'b0; //stop reset
	
	#300
	bot_in=11'b00111111100; //try sending data
	top_ready_in = 1;
	bot_valid_in = 1;

	#600
	bot_in=11'b10111111100; //try sending data
	#10
	bot_in=11'b00000001111; //try sending data to BD
	#10
	bot_in=11'b00000000001;
	#10
	bot_in=11'b00000000011;
	#10
	bot_in=11'b10000000111; //tail
	#10
	bot_in=11'b00000000000; //try sending data to BD
	#10
	bot_in=11'b00000001111;
	#10
	bot_in=11'b00000011111;
	#10
	bot_in=11'b10000111111;
	#10
	bot_valid_in = 0;
end

// BD src
BD_Source #(.NUM_BITS(34), .DelayMin(0), .DelayMax(200)) src(BD_in_data, _BD_in_valid, BD_in_ready, reset, BD_in_clk_ifc);
initial begin
	reset = 0;
	#15
	reset = 1;
	#15
	reset = 0;
end

//////////////////////////////////////////////////////////////
//CLKS

always 
	#15 osc =! osc;
	
always begin
	#5 //3x osc clk (50 vs 150)
	top_in_clk=!top_in_clk;
	bot_in_clk=!bot_in_clk;
	BD_in_clk_ifc=!BD_in_clk_ifc;
end

// always @ (posedge top_in_clk) begin
// 	if(~top_in_full) begin
// 		top_wr_in=1;
// 		top_test_input[9:0] = top_test_input[9:0]+1;
// 		top_packet_count = top_packet_count+1;
// 		if(top_packet_count==6) begin
// 			top_packet_count=0;
// 			top_test_input[10]=1;
// 		end
// 		else
// 			top_test_input[10]=0;
// 	end
// 	else
// 		top_wr_in=0;
// end


stack_core doot(.*);

endmodule // stack_core_tb