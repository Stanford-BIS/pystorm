`define SIMULATION
`include "../src/router/BZ_host_core.sv"

`include "BDSrcSink.sv"

`timescale 1ns/1ps
module BZ_host_core_tb();

reg osc; //signal for oscillator

//sys clks for ok
wire sys_clk_p;
assign sys_clk_p = osc;
wire sys_clk_n;
assign sys_clk_n = ~osc;

// OK ifc
wire [4:0]   okUH;
wire [2:0]   okHU;
wire [31:0]  okUHU;
wire         okAA;

wire [3:0]   led;

//input from other boards
reg top_in_clk;
reg top_valid_in;
reg top_ready_in;
reg [10:0] top_in;
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

//////////////////////////////////////////////////////////////
//CLKS

always #6 osc =! osc;

always begin
	#3 //3x osc clk (50 vs 150)
	top_in_clk=!top_in_clk;
end


BZ_host_core doot(.*);


//------------------------------------------------------------------------
// Begin okHostInterface simulation user configurable  global data
//------------------------------------------------------------------------
parameter BlockDelayStates = 5;   // REQUIRED: # of clocks between blocks of pipe data
parameter ReadyCheckDelay = 5;    // REQUIRED: # of clocks before block transfer before
                                  //           host interface checks for ready (0-255)
parameter PostReadyDelay = 5;     // REQUIRED: # of clocks after ready is asserted and
                                  //           check that the block transfer begins (0-255)
parameter pipeInSize = 16;         // REQUIRED: byte (must be even) length of default
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
const logic[31:0] nop = {2'b10, 6'd63, 24'd1}; // 6'd63 is the highest register, which is unused
const logic[31:0] nop_rt = {22'b0, 10'b1000000000}; //negative routes get discarded

// index into PipeIn
int i = 0;

task SetReg(logic [4:0] reg_id, logic[15:0] data);
	pipeInFlat[i] = {2'b10, 1'b0, reg_id, 8'b0, data};
	assert(i < pipeInSize);
	i = i + 1;
endtask

task SendPacket(logic[9:0] route, logic[31:0] payload);
	pipeInFlat[i] = payload;
	assert(i < pipeInSize);
	i = i + 1;
	pipeInFlat[i] = {22'b0, route};
	assert(i < pipeInSize);
	i = i + 1;
endtask

task FlushAndSendPipeIn();
	while (i + 1 < pipeInSize) begin
		pipeInFlat[i] = nop;
		i = i + 1;
		pipeInFlat[i] = nop_rt;
		i = i + 1;
	end
	WriteToBlockPipeIn(8'h80, pipeInSize, pipeInSize);
	for (i = 0; i < pipeInSize; i++) begin
		pipeInFlat[i] = 32'd0;
	end
	i = 0;
endtask


// OK program
initial begin
	osc = 0;
	top_in_clk = 0;
	top_in=11'b0;

	top_valid_in = 0; //not providing input data
	top_ready_in = 1; //always ready for output data

	user_reset <= 1;
	FrontPanelReset;  // Start routine with FrontPanelReset;
	user_reset <= 0;

  	// // turn off resets
  	// // ESSENTIAL: this test harness uses pReset/sReset for the BDSrc/Sink
  	// SetReg(31, 0); 
  	// FlushAndSendPipeIn(); // send the stuff we queued up

  	// send a packet
  	SendPacket(10'd3, 32'b01001100011100001111000001111100);
  	FlushAndSendPipeIn(); // send the stuff we queued up

  	#200
  	@(posedge top_in_clk) #6;
  	top_valid_in = 1;
	top_in=11'b00000100000; //send header
	#6
	top_in=11'b00000000001; //send data
	#6
	top_in=11'b00000000111;
	#6
	top_in=11'b00000011111;
	#6
	top_in=11'b0;
	top_valid_in = 0;

  	forever begin
  		#(1000)
  		ReadFromPipeOut(8'ha0, pipeOutSize);
  	end

end

`include "../ext/opalkelly/Simulation/okHostCalls.v"   // Do not remove!  The tasks, functions, and data stored
                                                       // in okHostCalls.v must be included here.

endmodule // stack_core_tb