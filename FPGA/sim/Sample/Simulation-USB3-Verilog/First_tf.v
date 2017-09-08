//------------------------------------------------------------------------
// First_tf.v
//
// A simple text fixture example for getting started with FrontPanel 3.x
// simulation.  This sample connects the top-level signals from First.v
// to a call system that, when integrated with Opal Kelly simulation
// libraries, mimics the functionality of FrontPanel.  Listed below are
// the tasks and functions that can be called.  They are designed to
// replicate calls made from the PC via FrontPanel API, Python, Java, DLL,
// etc.
//
//------------------------------------------------------------------------
// Copyright (c) 2005-2015 Opal Kelly Incorporated
// $Rev$ $Date$
//------------------------------------------------------------------------
`timescale 1ns / 1ps
`default_nettype none

module FIRST_TEST;

wire  [4:0]   okUH;
wire  [2:0]   okHU;
wire  [31:0]  okUHU;
wire  [7:0]   led;

First dut (
	.okUH(okUH),
	.okHU(okHU),
	.okUHU(okUHU),
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
parameter pipeInSize = 1024;      // REQUIRED: byte (must be even) length of default
                                  //           PipeIn; Integer 0-2^32
parameter pipeOutSize = 1024;     // REQUIRED: byte (must be even) length of default
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
reg [31:0] r1, r2, exp, sum;
initial r1 = 0;
initial r2 = 0;
initial exp = 0;
initial sum = 0;

initial begin
	FrontPanelReset;                      // Start routine with FrontPanelReset;

	//---------------------------------------------------------------------
	// Sample task and function operations
	//    We'll generate random numbers, send them to the DUT using
	//    simulated FrontPanel API calls and pull out the result of the 
	//    DUT 'add' function.  We will also automate the verification 
	//    process.
	//---------------------------------------------------------------------
	for (k=0; k<5; k=k+1) begin
		// Set the two ADDER inputs to random 16-bit values.
		r1 = $random & 32'hffff_ffff;
		r2 = $random & 32'hffff_ffff;
		exp = r1 + r2;
		SetWireInValue(8'h01, r1, 32'hffff_ffff);     // FRONTPANEL API
		SetWireInValue(8'h02, r2, 32'hffff_ffff);     // FRONTPANEL API
		UpdateWireIns;                           // FRONTPANEL API
		
		// The ADDER result will be ready.  UpdateWireOuts to get it.
		UpdateWireOuts;                          // FRONTPANEL API
		sum = GetWireOutValue(8'h21);            // FRONTPANEL API
		if (exp == sum)
			$display("SUCCESS -- Expected: 0x%04h   Received: 0x%04h", exp, sum);
		else
			$display("FAILURE -- Expected: 0x%04h   Received: 0x%04h", exp, sum);
	end

end

`include "./oksim/okHostCalls.v"   // Do not remove!  The tasks, functions, and data stored
                                   // in okHostCalls.v must be included here.

endmodule
