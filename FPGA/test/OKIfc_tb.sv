`define SIMULATION 

`include "../src/OKBD.sv"
`include "BDSrcSink.sv"

`timescale 1ns / 1ps
`default_nettype none

module OKIfc_tb;

parameter NPC = 32;
parameter NPCcode = 7;
parameter NPCroute = 5;
parameter NPCdata = 20;
parameter logic [NCroute-1:0] GO_HOME_rt = 31;
parameter logic [NPCcode-1:0] NOPcode = 64;
parameter logic [NPCcode-1:0] DSQueueCode = 65;

logic [NPCinout-1:0] nop_word;
assign nop_word = {GO_HOME_rt, NOPcode, {NPCdata{1'b0}}};

wire [4:0]   okUH;
wire [2:0]   okHU;
wire [31:0]  okUHU;
wire         okAA;
wire [3:0]   led;
wire [3:0]   led_in;
wire         okClk;
logic         user_reset;
Channel #(NPC) PC_downstream();
Channel #(NPC) PC_upstream();

// rename
logic clk;
assign clk = okClk;
logic reset;
assign reset = user_reset;


// DUT
OKIfc dut(.*);

// inner source/sink
ChannelSink downstream_sink(PC_downstream, clk, reset);

RandomChannelSrc #(NPC) upstream_source(PC_upstream, clk, reset);

//------------------------------------------------------------------------
// Begin okHostInterface simulation user configurable  global data
//------------------------------------------------------------------------
parameter block_size = 512;

parameter BlockDelayStates = 5;   // REQUIRED: # of clocks between blocks of pipe data
parameter ReadyCheckDelay = 5;    // REQUIRED: # of clocks before block transfer before
                                  //           host interface checks for ready (0-255)
parameter PostReadyDelay = 5;     // REQUIRED: # of clocks after ready is asserted and
                                  //           check that the block transfer begins (0-255)
parameter pipeInSize = 16 * 1024;         // REQUIRED: byte (must be even) length of default
                                          //           PipeIn; Integer 0-2^32
parameter pipeOutSize = 16 * 1024;        // REQUIRED: byte (must be even) length of default
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
const logic[NPC-1:0] ds_nop = nop_word;

// index into PipeIn
int i = 0;

task SendToFPGA(logic [31:0] data);
  pipeInFlat[i] = data;
  assert(i < pipeInSize);
  i = i + 1;
endtask

task FlushAndSendPipeIn(int send_size);
  assert(send_size < pipeInSize);
  assert(send_size > i);

  // pad nops
  while (i < send_size) begin
    pipeInFlat[i] = ds_nop;
    i = i + 1;
  end

  // send padded data
  WriteToBlockPipeIn(8'h80, block_size, send_size);

  // clear buffer
  for (i = 0; i < pipeInSize; i++) begin
    pipeInFlat[i] = 0;
  end
  i = 0;
endtask


localparam int send_data_size = 3 * block_size / 2;
localparam int send_nop_size  = 2 * block_size - send_data_size;
localparam int send_size = send_data_size + send_nop_size;

logic [31:0] rand_data;

// OK program
initial begin
  user_reset <= 1;
	FrontPanelReset;                      // Start routine with FrontPanelReset;
  user_reset <= 0;


  #(1000)
  forever begin

    for (int j = 0; j < send_data_size; j++) begin
      rand_data = $urandom_range(0, 2**32-1);
      SendToFPGA(rand_data);
    end
    FlushAndSendPipeIn(send_size); // send the stuff we queued up
    #(1000)
    ReadFromBlockPipeOut(8'ha0, block_size, 3 * block_size);
  end

end

`include "../ext/opalkelly/Simulation/okHostCalls.v"   // Do not remove!  The tasks, functions, and data stored
                                                       // in okHostCalls.v must be included here.

endmodule
