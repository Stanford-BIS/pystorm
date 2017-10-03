`define SIMULATION 

`include "../src/OKCoreBD.sv"
`include "BDSrcSink.sv"

`timescale 1ns / 1ps
`default_nettype none

module OKCoreBD_tb;

// OK ifc
wire [4:0]   okUH;
wire [2:0]   okHU;
wire [31:0]  okUHU;
wire         okAA;

wire [3:0]   led;

// BD ifc
wire        BD_out_clk;
wire        BD_out_ready;
wire        BD_out_valid;
wire [20:0] BD_out_data;

wire        BD_in_clk;
wire        BD_in_ready;
wire        _BD_in_valid;
wire[33:0]  BD_in_data;

wire        pReset;
wire        sReset;

wire        adc0;
wire        adc1;

// external clock, drives PLL to generate BD IO clocks
logic sys_clk_p;
parameter Tsys_clk_p = 10;
always #(Tsys_clk_p/2) sys_clk_p = ~sys_clk_p;
initial sys_clk_p = 0;

logic sys_clk_n;
assign sys_clk_n = ~sys_clk_p;

// external reset
logic user_reset;
initial begin
  user_reset <= 0;
  #(10) user_reset <= 1;
  #(100) user_reset <= 0;
end

// DUT
OKCoreBD dut(.*);

logic reset_for_BD_src_sink;
assign reset_for_BD_src_sink = sReset | pReset;
// BD src
BD_Source #(.NUM_BITS(34), .DelayMin(0), .DelayMax(200)) src(BD_in_data, _BD_in_valid, BD_in_ready, reset_for_BD_src_sink, BD_in_clk);

// BD sink
BD_Sink #(.NUM_BITS(21), .DelayMin(0), .DelayMax(200)) sink(BD_out_ready, BD_out_valid, BD_out_data, reset_for_BD_src_sink, BD_out_clk);

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

// index into PipeIn
int i = 0;

task SetReg(logic [4:0] reg_id, logic[15:0] data);
  pipeInFlat[i] = {2'b10, 1'b0, reg_id, 8'b0, data};
  assert(i < pipeInSize);
  i = i + 1;
endtask

task SetChan(logic [4:0] chan_id, logic[15:0] data);
  pipeInFlat[i] = {2'b11, 1'b0, chan_id, 8'b0, data};
  assert(i < pipeInSize);
  i = i + 1;
endtask

task SendToBD(logic[5:0] code, logic[23:0] payload);
  pipeInFlat[i] = {2'b00, code, payload};
  assert(i < pipeInSize);
  i = i + 1;
endtask

task SendFromBD(logic[33:0] payload);
  pipeInFlat[i] = {8'hff, payload[23:0]};
  assert(i < pipeInSize);
  i = i + 1;
  pipeInFlat[i] = {8'hff, 14'd0, payload[33:24]};
  assert(i < pipeInSize);
  i = i + 1;
endtask

task FlushAndSendPipeIn();
  while (i < pipeInSize) begin
    pipeInFlat[i] = nop;
    i = i + 1;
  end
  WriteToBlockPipeIn(8'h80, pipeInSize, pipeInSize);
  for (i = 0; i < pipeInSize; i++) begin
    pipeInFlat[i] = 32'd0;
  end
  i = 0;
endtask

task SendToAllBD(int start, int num_words);
  localparam NumHornLeaves = 34;
  for (int i = 0; i < num_words; i++) begin
    automatic logic [5:0] leaf = (start + i) % NumHornLeaves;
    automatic logic [23:0] payload = $urandom_range(0, 2**23-1);
    SendToBD({2'b00, leaf}, payload);
  end
endtask


// OK program
initial begin
  user_reset <= 1;
	FrontPanelReset;                      // Start routine with FrontPanelReset;
  user_reset <= 0;

  // turn off resets
  // ESSENTIAL: this test harness uses pReset/sReset for the BDSrc/Sink
  SetReg(31, 0); 
  FlushAndSendPipeIn(); // send the stuff we queued up

  // send AM word
  SendToBD({2'b00, 6'd26}, {3{8'b11110000}});
  SendToBD({2'b00, 6'd26}, {3{8'b11110000}});
  FlushAndSendPipeIn(); // send the stuff we queued up

  // send PAT word
  SendToBD({2'b00, 6'd27}, {3{8'b11110000}});
  SendToBD({2'b00, 6'd27}, {3{8'b11110000}});
  FlushAndSendPipeIn(); // send the stuff we queued up

  // send TAT0 word
  SendToBD({2'b00, 6'd28}, {3{8'b11110000}});
  SendToBD({2'b00, 6'd28}, {3{8'b11110000}});
  FlushAndSendPipeIn(); // send the stuff we queued up

  // send TAT1 word
  SendToBD({2'b00, 6'd29}, {3{8'b11110000}});
  SendToBD({2'b00, 6'd29}, {3{8'b11110000}});
  FlushAndSendPipeIn(); // send the stuff we queued up

  //SendToBD(0, 3'b101); // ADC 
  //SendToBD(1, 11'b10101010101); // DAC0


  //#(1000)
  //ReadFromPipeOut(8'ha0, pipeOutSize); // get inputs from BDsrc

  //#(1000)
  //ReadFromPipeOut(8'ha0, pipeOutSize); // get inputs from BDsrc

  //// send a bunch of BD words
  //// do it a few times in case pipeInSize < number of horn leaves (34)

  //SendToAllBD(0*pipeInSize, pipeInSize); 
  //FlushAndSendPipeIn();
  //SendToAllBD(1*pipeInSize, pipeInSize);
  //FlushAndSendPipeIn();
  //SendToAllBD(2*pipeInSize, pipeInSize);
  //FlushAndSendPipeIn();

  forever begin
    #(1000)
    ReadFromPipeOut(8'ha0, pipeOutSize);
  end

end

`include "../ext/opalkelly/Simulation/okHostCalls.v"   // Do not remove!  The tasks, functions, and data stored
                                                       // in okHostCalls.v must be included here.

endmodule
