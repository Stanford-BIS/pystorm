// Testbench for OKCoreHarness (tests FPGA logic without BD handshake)
// uses OK behavioral models for the okHost and endpoints
// adapated from First.v, following Sample/Simulation-USB3-Verilog/README.txt

`timescale 1ns / 1ps
`default_nettype none

module OKCoreHarness_tb;

wire  [4:0]   okUH;
wire  [2:0]   okHU;
wire          okAA;
wire  [31:0]  okUHU;
wire  [3:0]   led;

logic user_reset;

OKCoreTestHarness dut (
	.okUH(okUH),
	.okHU(okHU),
  .okAA(okAA),
	.okUHU(okUHU),
  .user_reset(user_reset),
	.led(led)
	);


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

//------------------------------------------------------------------------
//  Available User Task and Function Calls:
//    FrontPanelReset;                  // Always start routine with FrontPanelReset;
//    SetWireInValue(ep, val, mask);
//    UpdateWireIns;
//    UpdateWireOuts;
//    GetWireOutValue(ep);
//    ActivateTriggerIn(ep, bit);       // bit is an integer 0-15
//    UpdateTriggerOuts;
//    IsTriggered(ep, mask);            // Returns a 1 or 0
//    WriteToPipeIn(ep, length);        // passes pipeIn array data
//    ReadFromPipeOut(ep, length);      // passes data to pipeOut array
//    WriteToBlockPipeIn(ep, blockSize, length);    // pass pipeIn array data; blockSize and length are integers
//    ReadFromBlockPipeOut(ep, blockSize, length);  // pass data to pipeOut array; blockSize and length are integers
//
//    *Pipes operate by passing arrays of data back and forth to the user's
//    design.  If you need multiple arrays, you can create a new procedure
//    above and connect it to a differnet array.  More information is
//    available in Opal Kelly documentation and online support tutorial.
//------------------------------------------------------------------------

// User configurable block of called FrontPanel operations.

// easier to work in 32bit chunks
logic [(pipeInSize/4)-1:0][31:0] pipeInFlat;
assign {<<8{pipeIn}} = pipeInFlat;

// functions for creating downstream words
const logic[31:0] nop = {2'b10, 6'd31, 24'd1}; // 6'd31 is the highest register, which is unused

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

task SendToBD(logic[5:0] code, logic[19:0] payload);
  pipeInFlat[i] = {2'b00, code, 4'b0, payload};
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

initial begin
  user_reset <= 1;
	FrontPanelReset;                      // Start routine with FrontPanelReset;
  user_reset <= 0;

  SendToBD(0, 3'b101); // ADC 
  SendToBD(1, 11'b10101010101); // DAC0
  FlushAndSendPipeIn(); // send the stuff we queued up

  ReadFromPipeOut(8'ha0, pipeOutSize); // get the messages we sent into BD

  SendFromBD({2'b00, {16{2'b10}}}); // crazy TAT word
  FlushAndSendPipeIn(); // send the stuff we queued up

  ReadFromPipeOut(8'ha0, pipeOutSize); // get the messages we sent into BD

  #(1000)
  ReadFromPipeOut(8'ha0, pipeOutSize); // get the messages we sent into BD

  #(1000)
  ReadFromPipeOut(8'ha0, pipeOutSize); // get the messages we sent into BD

end

`include "./okModels/okHostCalls.v"   // Do not remove!  The tasks, functions, and data stored
                                      // in okHostCalls.v must be included here.

endmodule
