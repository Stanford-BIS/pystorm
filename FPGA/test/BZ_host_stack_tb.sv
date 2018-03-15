`define SIMULATION
`include "../src/router/BZ_host_core.sv"

`include "BDSrcSink.sv"

`timescale 1ns/1ps
module BZ_host_stack_tb();

//host core

reg osc; //signal for oscillator
reg osc_host; //signal for oscillator

//sys clks for ok
wire sys_clk_p;
assign sys_clk_p = osc_host;
wire sys_clk_n;
assign sys_clk_n = ~osc_host;

// OK ifc
wire [4:0]   okUH;
wire [2:0]   okHU;
wire [31:0]  okUHU;
wire         okAA;

wire [3:0]   led;

reg [10:0] top_out_stack;
wire [10:0] bot_out;
reg top_valid_out_stack;
wire bot_valid_out;
reg top_ready_out_stack;
wire bot_ready_out;
reg top_out_clk_stack;
wire bot_out_clk;

//input from other boards
wire top_in_clk = bot_out_clk;
wire top_valid_in = bot_valid_out;
wire top_ready_in = bot_ready_out;
wire [10:0] top_in = bot_out;
reg [10:0] top_out;
reg top_valid_out;
reg top_ready_out;
reg top_out_clk;

reg user_reset = 0;

initial begin
	user_reset = 0;
	#10
	user_reset = 1;
	#100
	user_reset = 0;
end

//-------------------------------------

//stack core

//input from other boards
reg top_in_clk_stack;
wire bot_in_clk = top_out_clk;
reg top_valid_in_stack;
wire bot_valid_in = top_valid_out;
reg top_ready_in_stack;
wire bot_ready_in = top_ready_out;
reg [10:0] top_in_stack;
wire [10:0] bot_in = top_out;


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


//////////////////////////////////////////////////////////////
//CLKS

always 
	#6 osc =! osc;

always 
	#3 osc_host =! osc_host;

always begin
	#2 //3x osc clk (50 vs 150)
	top_in_clk_stack=!top_in_clk_stack;
end
	

BZ_host_core doot(.*);
stack_core thank(
	.top_in_clk    (top_in_clk_stack),
	.top_in        (top_in_stack),
	.top_out       (top_out_stack),
	.top_valid_in  (top_valid_in_stack),
	.top_ready_in  (top_ready_in_stack),
	.top_valid_out (top_valid_out_stack),
	.top_ready_out (top_ready_out_stack),
	.top_out_clk   (top_out_clk_stack),.*);

// BD src
BD_Source #(.NUM_BITS(34), .DelayMin(0), .DelayMax(200)) src(BD_in_data, _BD_in_valid, BD_in_ready, user_reset, BD_in_clk_ifc);
// BD sink
BD_Sink #(.NUM_BITS(21), .DelayMin(0), .DelayMax(200)) sink(BD_out_ready, BD_out_valid, BD_out_data, user_reset, BD_out_clk);


//------------------------------------------------------------------------
// Begin okHostInterface simulation user configurable  global data
//------------------------------------------------------------------------
parameter BlockDelayStates = 5;   // REQUIRED: # of clocks between blocks of pipe data
parameter ReadyCheckDelay = 5;    // REQUIRED: # of clocks before block transfer before
                                  //           host interface checks for ready (0-255)
parameter PostReadyDelay = 5;     // REQUIRED: # of clocks after ready is asserted and
                                  //           check that the block transfer begins (0-255)
parameter pipeInSize = 32;         // REQUIRED: byte (must be even) length of default
                                  //           PipeIn; Integer 0-2^32
parameter pipeOutSize = 16;        // REQUIRED: byte (must be even) length of default
                                  //           PipeOut; Integer 0-2^32

integer k;
reg  [7:0]  pipeIn [0:(pipeInSize-1)];
initial for (k=0; k<pipeInSize; k=k+1) pipeIn[k] = 8'h00;

reg  [7:0]  pipeOut [0:(pipeOutSize-1)];
initial for (k=0; k<pipeOutSize; k=k+1) pipeOut[k] = 8'h00;

wire [31:0] u32Address [0:31];
reg  [31:0] u32Data [0:31];
wire [31:0] u32Count;
wire [31:0] ReadRegisterData;

// easier to work in 32bit chunks
logic [(pipeInSize/4)-1:0][31:0] pipeInFlat;
assign {<<8{pipeIn}} = pipeInFlat;

// functions for creating downstream words
const logic[31:0] nop = {5'b10000, 27'd0}; // 6'd63 is the highest register, which is unused

// index into PipeIn
int i = 0;

task SetReg(logic [4:0] reg_id, logic[15:0] data);
	pipeInFlat[i] = {2'b10, 1'b0, reg_id, 8'b0, data};
	assert(i < pipeInSize);
	i = i + 1;
endtask

task SendPacket(logic[31:0] payload);
	pipeInFlat[i] = payload;
	assert(i < pipeInSize);
	i = i + 1;
endtask

task SendToEP(logic [6:0] ep_id, logic[19:0] data);
  pipeInFlat[i] = {5'b00001, ep_id, data};
  assert(i < pipeInSize);
  i = i + 1;
endtask

task FlushAndSendPipeIn();
	while (i + 1 < pipeInSize) begin
		pipeInFlat[i] = nop;
		i = i + 1;
	end
	WriteToBlockPipeIn(8'h80, pipeInSize, pipeInSize);
	for (i = 0; i < pipeInSize; i++) begin
		pipeInFlat[i] = 32'd0;
	end
	i = 0;
endtask

const logic [7:0] gen_idx_fast = 0;
const logic [7:0] gen_idx_slow = 1;
const logic [15:0] period_fast = 1;
const logic [15:0] period_slow = 4;
const logic [15:0] ticks = 0;
const logic [10:0] tag = 0;
const logic sign_fast = 0;
const logic sign_slow = 1;

const logic [63:0] SG_word_fast = {sign_fast, gen_idx_fast, period_fast, ticks, tag};
const logic [63:0] SG_word_slow = {sign_slow, gen_idx_slow, period_slow, ticks, tag};

const logic [3:0][15:0] SG_word_fast_pieces = SG_word_fast;
const logic [3:0][15:0] SG_word_slow_pieces = SG_word_slow;

logic [19:0] send_val;
// OK program
initial begin
	osc = 0;
	osc_host = 0;

	top_valid_in_stack = 0; //not providing input data
	top_ready_in_stack = 1; //always ready for output data

	user_reset <= 1;
	FrontPanelReset;  // Start routine with FrontPanelReset;
	user_reset <= 0;

  	// turn off resets
  	// ESSENTIAL: this test harness uses pReset/sReset for the BDSrc/Sink
  	// SetReg(31, 0); 
  	// FlushAndSendPipeIn(); // send the stuff we queued up

  	// send a packet
  	// send_val = 20'b0;
  	// repeat(256) begin
  	// 	SendPacket({5'b00001, 7'd26, send_val});
  	// 	send_val = send_val + 20'b1;
  	// end
  	// SendPacket({5'b00001, 7'd26, 20'b0});
  	// FlushAndSendPipeIn(); // send the stuff we queued up

	  	  // program SG
	  SendToEP(7'd69, {4'd0, 16'd1}); // gens used
	  SendToEP(7'd70, {4'd0, 16'd1}); // enable
	  SendToEP(7'd112, {4'd0, SG_word_fast_pieces[0]});
	  SendToEP(7'd112, {4'd0, SG_word_fast_pieces[1]});
	  SendToEP(7'd112, {4'd0, SG_word_fast_pieces[2]});
	  SendToEP(7'd112, {4'd0, SG_word_fast_pieces[3]});
	  FlushAndSendPipeIn(); // send the stuff we queued up

	  	  	  // program SF
	  SendToEP(7'd64, {4'd0, 16'd1}); // gens used
	  SendToEP(7'd65, {4'd0, 16'd1}); // increment
	  FlushAndSendPipeIn(); // send the stuff we queued up

	  //send a really late hb
	  SendToEP(7'd87, {20{1'b1}});
	  SendToEP(7'd88, {20{1'b1}});
	  SendToEP(7'd89, {20{1'b1}});
	  SendPacket({5'b00001, 7'd26, 20'd1});
	  SendPacket({5'b00001, 7'd26, 20'd2});
	  SendPacket({5'b00001, 7'd26, 20'd3});
	  FlushAndSendPipeIn(); // send the stuff we queued up


  	forever begin
  		#(1000)
  		ReadFromPipeOut(8'ha0, pipeOutSize);
  	end

end

`include "../ext/opalkelly/Simulation/okHostCalls.v"   // Do not remove!  The tasks, functions, and data stored
                                                       // in okHostCalls.v must be included here.

endmodule // stack_core_tb