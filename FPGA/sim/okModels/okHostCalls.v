//------------------------------------------------------------------------
// okHostCalls.v
//
// Description:
//    This file is included by a test fixture designed to mimic FrontPanel
//    operations.  The functions and task below provide a pseudo
//    translation between the FrontPanel operations and the hi_cmd, hi_out,
//    and hi_inout signals.
//------------------------------------------------------------------------
// Copyright (c) 2005-2010 Opal Kelly Incorporated
// $Rev$ $Date$
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// *  Do not edit any of the defines, registers, integers, arrays, or
//    functions below this point.
// *  Tasks in Verilog cannot pass arrays.  The pipe tasks utilize arrays
//    of data. If you require multiple pipe arrays, you may create new
//    arrays in the top level file (that `includes this file), duplicate
//    the pipe tasks below as required, change the names of the duplicated
//    tasks to unique identifiers, and alter the pipe array in those tasks
//    to your newly generated arrays in the top level file.
// *  For example, in the top level file, along with:
//       reg   [7:0] pipeIn [0:(pipeInSize-1)];
//       reg   [7:0] pipeOut [0:(pipeOutSize-1)];
//       - Add:   reg   [7:0] pipeIn2 [0:1023];
//       - Then, copy the WriteToPipeIn task here, rename it WriteToPipeIn2,
//         and finally change pipeIn[i] in WriteToPipeIn2 to pipeIn2[i].
//    The task and operation can then be called with a:
//       WriteToPipeIn2(8'h80, 1024);//endpoint 0x80 pipe received pipeIn2
//------------------------------------------------------------------------
`include "parameters.v"
                 
`define DNOP                  3'h0
`define DReset                3'h1
`define DWires                3'h2
`define DUpdateWireIns        3'h1
`define DUpdateWireOuts       3'h2
`define DTriggers             3'h3
`define DActivateTriggerIn    3'h1
`define DUpdateTriggerOuts    3'h2
`define DPipes                3'h4
`define DWriteToPipeIn        3'h1
`define DReadFromPipeOut      3'h2
`define DWriteToBlockPipeIn   3'h3
`define DReadFromBlockPipeOut 3'h4
`define DRegisters            3'h5
`define DWriteRegister        3'h1
`define DReadRegister         3'h2
`define DWriteRegisterSet     3'h3
`define DReadRegisterSet      3'h4

// Local okHostCall signals
reg            hi_clk;
reg            hi_drive;
reg   [2:0]    hi_cmd;
wire           hi_busy;
wire  [31:0]   hi_datain;
reg   [31:0]   hi_dataout;

reg   [31:0]   WireIns [0:31];   // 32x32 array storing WireIn values
reg   [31:0]   WireOuts [0:31];  // 32x32 array storing WireOut values
reg   [31:0]   Triggered [0:31]; // 32x32 array storing IsTriggered values

initial begin
	hi_clk     = 1'b0;
	hi_drive   = 1'b0;
	hi_cmd     = `DNOP;
	hi_dataout = 32'h0000;
end

// Mapping of local okHostCall signals to okHost interface
assign okUH[0]   = hi_clk;
assign okUH[1]   = hi_drive;
assign okUH[4:2] = hi_cmd; 
assign hi_datain = okUHU;
assign hi_busy   = okHU[0]; 
assign okUHU     = hi_drive ? hi_dataout : 32'hzzzz;

// Clock Generation
parameter tCK = 5; // Half of the hi_clk frequency @ 1ns timing
always #tCK hi_clk = ~hi_clk;
	
//---------------------------------------------------------
// FrontPanelReset
//---------------------------------------------------------
task FrontPanelReset ();
	integer i;
begin
	for (i=0; i<32; i=i+1) begin
		WireIns[i] = 32'h0000;
		WireOuts[i] = 32'h0000;
		Triggered[i] = 32'h0000;
	end
	
	@(posedge hi_clk) hi_cmd[2:0] = `DReset;
	@(posedge hi_clk) hi_cmd[2:0] = `DNOP;
	wait (hi_busy == 0);
end
endtask

//---------------------------------------------------------
// SetWireInValue
//---------------------------------------------------------
task SetWireInValue (
	input    [7:0]    ep,
	input    [31:0]   val,
	input    [31:0]   mask
);
	reg   [31:0]   tmp;
begin
	tmp = WireIns[ep] & ~mask;
	WireIns[ep] = tmp | (val & mask);
end
endtask

//---------------------------------------------------------
// GetWireOutValue
//---------------------------------------------------------
function [31:0] GetWireOutValue (
	input    [7:0]    ep
);
begin
	GetWireOutValue = WireOuts[ep - 8'h20];
end
endfunction

//---------------------------------------------------------
// IsTriggered
//---------------------------------------------------------
function IsTriggered (
	input    [7:0]    ep,
	input    [31:0]   mask
);
begin
	if ((Triggered[ep - 8'h60] & mask) >= 0) begin
		if ((Triggered[ep - 8'h60] & mask) == 0) begin
			IsTriggered = 0;
		end else begin
			IsTriggered = 1;
		end
	end else begin
		$display("***FRONTPANEL ERROR: IsTriggered mask 0x%04h covers unused Triggers", mask);
		IsTriggered = 0;
	end
end
endfunction

//---------------------------------------------------------
// UpdateWireIns
//---------------------------------------------------------
task UpdateWireIns ();
   integer i;
begin
	@(posedge hi_clk) hi_cmd[2:0] = `DWires;
	@(posedge hi_clk) hi_cmd[2:0] = `DUpdateWireIns;
	@(posedge hi_clk);
	hi_drive = 1;
	@(posedge hi_clk) hi_cmd[2:0] = `DNOP;
	for (i=0; i<32; i=i+1) begin
		hi_dataout = WireIns[i];
		@(posedge hi_clk) ;
	end
	wait (hi_busy == 0);
end
endtask

//---------------------------------------------------------
// UpdateWireOuts
//---------------------------------------------------------
task UpdateWireOuts ();
	integer i;
begin
	@(posedge hi_clk) hi_cmd[2:0] = `DWires;
	@(posedge hi_clk) hi_cmd[2:0] = `DUpdateWireOuts;
	@(posedge hi_clk);
	@(posedge hi_clk) hi_cmd[2:0] = `DNOP;
	@(posedge hi_clk) hi_drive = 0;
	@(posedge hi_clk); @(posedge hi_clk);
	for (i=0; i<32; i=i+1)
		@(posedge hi_clk) WireOuts[i] = hi_datain;
	wait (hi_busy == 0);
end
endtask

//---------------------------------------------------------
// ActivateTriggerIn
//---------------------------------------------------------
task ActivateTriggerIn (
	input    [7:0]    ep,
	input    [31:0]  trig_bit
);
begin
	@(posedge hi_clk) hi_cmd[2:0] = `DTriggers;
	@(posedge hi_clk) hi_cmd[2:0] = `DActivateTriggerIn;
	@(posedge hi_clk);
	hi_drive = 1;
	hi_dataout = {24'h00, ep};
	@(posedge hi_clk) hi_dataout = (1'b1 << trig_bit);
	hi_cmd[2:0] = `DNOP;
	@(posedge hi_clk) hi_dataout = 32'h0000;
	wait (hi_busy == 0);
end
endtask

//---------------------------------------------------------
// UpdateTriggerOuts
//---------------------------------------------------------
task UpdateTriggerOuts ();
	integer i;
begin
	@(posedge hi_clk) hi_cmd[2:0] = `DTriggers;
	@(posedge hi_clk) hi_cmd[2:0] = `DUpdateTriggerOuts;
	@(posedge hi_clk);
	@(posedge hi_clk) hi_cmd[2:0] = `DNOP;
	@(posedge hi_clk) hi_drive = 0;
	@(posedge hi_clk); @(posedge hi_clk); @(posedge hi_clk);
	
	for (i=0; i<UPDATE_TO_READOUT_CLOCKS; i=i+1)@(posedge hi_clk);
	
	for (i=0; i<32; i=i+1)
		@(posedge hi_clk) Triggered[i] = hi_datain;
	wait (hi_busy == 0);
end
endtask


//---------------------------------------------------------
// WriteToPipeIn
//---------------------------------------------------------
task WriteToPipeIn (
	input    [7:0]    ep,
	input    [31:0]   length
);
	integer  len, i, j, k, blockSize;
begin
	len = length/4; j = 0; blockSize = 1024;
	if (length%2)
		$display("Error. Pipes commands may only send and receive an even # of bytes.");
	@(posedge hi_clk) hi_cmd[2:0] = `DPipes;
	@(posedge hi_clk) hi_cmd[2:0] = `DWriteToPipeIn;
	@(posedge hi_clk);
	hi_drive = 1;
	hi_dataout = {BlockDelayStates, ep};
	@(posedge hi_clk) hi_cmd[2:0] = `DNOP;
	hi_dataout = len;
	for (i=0; i < length; i=i+4) begin
		@(posedge hi_clk);
		hi_dataout[7:0]   = pipeIn[i];
		hi_dataout[15:8]  = pipeIn[i+1];
		hi_dataout[23:16] = pipeIn[i+2];
		hi_dataout[31:24] = pipeIn[i+3];
		j=j+4;
		if (j == blockSize) begin
			for (k=0; k < BlockDelayStates; k=k+1) begin
				@(posedge hi_clk);
			end
			j=0;
		end
	end
	wait (hi_busy == 0);
end
endtask


//---------------------------------------------------------
// ReadFromPipeOut
//---------------------------------------------------------
task ReadFromPipeOut (
	input    [7:0]    ep,
	input    [31:0]   length
);
	integer len, i, j, k, blockSize;
begin
	len = length/4; j = 0; blockSize = 1024;
	if (length%2)
		$display("Error. Pipes commands may only send and receive an even # of bytes.");
	@(posedge hi_clk) hi_cmd[2:0] = `DPipes;
	@(posedge hi_clk) hi_cmd[2:0] = `DReadFromPipeOut;
	@(posedge hi_clk);
	hi_drive = 1;
	hi_dataout = {BlockDelayStates, ep};
	@(posedge hi_clk) hi_cmd[2:0] = `DNOP;
	hi_dataout = len;
	@(posedge hi_clk) hi_drive = 0;
	for (i=0; i < length; i=i+4) begin
		@(posedge hi_clk);
		pipeOut[i]   = hi_datain[7:0];
		pipeOut[i+1] = hi_datain[15:8];
		pipeOut[i+2] = hi_datain[23:16];
		pipeOut[i+3] = hi_datain[31:24];
		j=j+4;
		if (j == blockSize) begin
			for (k=0; k < BlockDelayStates; k=k+1) begin
				@(posedge hi_clk);
			end
			j=0;
		end
	end
	wait (hi_busy == 0);
end
endtask

//---------------------------------------------------------
// WriteToBlockPipeIn
//---------------------------------------------------------
task WriteToBlockPipeIn (
	input    [7:0]    ep,
	input    [31:0]   blockLength,
	input    [31:0]   length
);
	integer len, blockSize, blockNum, i, j, k;
begin
	len = length/4; blockSize = blockLength/4; k = 0;
	blockNum = len/blockSize;
	if (length%2)
		$display("Error. Pipes commands may only send and receive an even # of bytes.");
	if (blockLength%2)
		$display("Error. Block Length may only be an even # of bytes.");
	if (length%blockLength)
		$display("Error. Pipe length MUST be a multiple of block length!");
	@(posedge hi_clk) hi_cmd[2:0] = `DPipes;
	@(posedge hi_clk) hi_cmd[2:0] = `DWriteToBlockPipeIn;
	@(posedge hi_clk);
	hi_drive = 1;
	hi_dataout = {BlockDelayStates, ep};
	@(posedge hi_clk) hi_cmd[2:0] = `DNOP;
	hi_dataout = len;
	@(posedge hi_clk) hi_dataout = blockSize;
	@(posedge hi_clk) hi_dataout[7:0] = ReadyCheckDelay; hi_dataout[15:8] = PostReadyDelay;
	for (i=0; i < blockNum; i=i+1) begin
		while (hi_busy === 1) @(posedge hi_clk);
		while (hi_busy === 0) @(posedge hi_clk);
		@(posedge hi_clk); @(posedge hi_clk);
		for (j=0; j<blockSize; j=j+1) begin
			hi_dataout[7:0]   = pipeIn[k]; 
			hi_dataout[15:8]  = pipeIn[k+1];
			hi_dataout[23:16] = pipeIn[k+2];
			hi_dataout[31:24] = pipeIn[k+3];
			@(posedge hi_clk); k=k+4;
		end
		for (j=0; j < BlockDelayStates; j=j+1) @(posedge hi_clk);
	end
	wait (hi_busy == 0);
end
endtask

//---------------------------------------------------------
// ReadFromBlockPipeOut
//---------------------------------------------------------
task ReadFromBlockPipeOut (
	input    [7:0]    ep,
	input    [31:0]   blockLength,
	input    [31:0]   length
);
   integer len, blockSize, blockNum, i, j, k;
begin
	len = length/4; blockSize = blockLength/4; k = 0;
	blockNum = len/blockSize;
	if (length%2)
		$display("Error. Pipes commands may only send and receive an even # of bytes.");
	if (blockLength%2)
		$display("Error. Block Length may only be an even # of bytes.");
	if (length%blockLength)
		$display("Error. Pipe length MUST be a multiple of block length!");
	@(posedge hi_clk) hi_cmd[2:0] = `DPipes;
	@(posedge hi_clk) hi_cmd[2:0] = `DReadFromBlockPipeOut;
	@(posedge hi_clk);
	hi_drive = 1;
	hi_dataout = {BlockDelayStates, ep};
	@(posedge hi_clk) hi_cmd[2:0] = `DNOP;
	hi_dataout = len;
	@(posedge hi_clk) hi_dataout = blockSize;
	@(posedge hi_clk) hi_dataout[7:0] = ReadyCheckDelay; hi_dataout[15:8] = PostReadyDelay;
	@(posedge hi_clk) hi_drive = 0;
	for (i=0; i < blockNum; i=i+1) begin
		while (hi_busy === 1) @(posedge hi_clk);
		while (hi_busy === 0) @(posedge hi_clk);
		@(posedge hi_clk); @(posedge hi_clk);
		for (j=0; j<blockSize; j=j+1) begin
			pipeOut[k]   = hi_datain[7:0]; 
			pipeOut[k+1] = hi_datain[15:8];
			pipeOut[k+2] = hi_datain[23:16];
			pipeOut[k+3] = hi_datain[31:24];
			@(posedge hi_clk) k=k+4;
		end
		for (j=0; j < BlockDelayStates; j=j+1) @(posedge hi_clk);
	end
	wait (hi_busy == 0);
end
endtask

//---------------------------------------------------------
// WriteRegister
//---------------------------------------------------------
task WriteRegister (
	input    [31:0]   address,
	input    [31:0]   data
);
begin
	@(posedge hi_clk) hi_cmd[2:0] = `DRegisters;
	@(posedge hi_clk) hi_cmd[2:0] = `DWriteRegister;
	@(posedge hi_clk);
	hi_drive = 1;
	hi_cmd[2:0] = `DNOP;
	@(posedge hi_clk) hi_dataout = address;
	@(posedge hi_clk) hi_dataout = data;
	wait (hi_busy == 0); hi_drive = 0; 
end
endtask

//---------------------------------------------------------
// ReadRegister
//---------------------------------------------------------
task ReadRegister (
	input    [31:0]   address,
	output   [31:0]   data
);
begin
	@(posedge hi_clk) hi_cmd[2:0] = `DRegisters;
	@(posedge hi_clk) hi_cmd[2:0] = `DReadRegister;
	@(posedge hi_clk);
	hi_drive = 1;
	hi_cmd[2:0] = `DNOP;
	@(posedge hi_clk) hi_dataout = address;
	@(posedge hi_clk); hi_drive = 0; 
	@(posedge hi_clk);
	@(posedge hi_clk) data = hi_datain;
	wait (hi_busy == 0);
end
endtask

//---------------------------------------------------------
// WriteRegisterSet
//---------------------------------------------------------
task WriteRegisterSet ();
	integer i;
begin
	@(posedge hi_clk) hi_cmd[2:0] = `DRegisters;
	@(posedge hi_clk) hi_cmd[2:0] = `DWriteRegisterSet;
	@(posedge hi_clk);
	hi_drive = 1;
	hi_cmd[2:0] = `DNOP;
	@(posedge hi_clk); hi_dataout = u32Count;
	for (i=0; i < u32Count; i=i+1) begin
		@(posedge hi_clk) hi_dataout = u32Address[i];
		@(posedge hi_clk) hi_dataout = u32Data[i];
		@(posedge hi_clk); @(posedge hi_clk); 
	end
	wait (hi_busy == 0); hi_drive = 0; 
end
endtask

//---------------------------------------------------------
// ReadRegisterSet
//---------------------------------------------------------
task ReadRegisterSet ();
	integer i;
begin
	@(posedge hi_clk) hi_cmd[2:0] = `DRegisters;
	@(posedge hi_clk) hi_cmd[2:0] = `DReadRegisterSet;
	@(posedge hi_clk);
	hi_drive = 1;
	hi_cmd[2:0] = `DNOP;
	@(posedge hi_clk); hi_dataout = u32Count;
	for (i=0; i < u32Count; i=i+1) begin
		@(posedge hi_clk) hi_dataout = u32Address[i];
		@(posedge hi_clk); hi_drive = 0; 
		@(posedge hi_clk);
		@(posedge hi_clk) u32Data[i] = hi_datain;
		hi_drive = 1;
	end
	wait (hi_busy == 0);
end
endtask