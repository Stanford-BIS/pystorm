//------------------------------------------------------------------------
// okHost.v
//
//  Description:
//    This file is a simulation replacement for okHost/okLibrary for
//    FrontPanel. It receives data from okHostCalls.v which is 
//    then restructured and timed to communicate with the endpoint
//    simulation modules.
//------------------------------------------------------------------------
// Copyright (c) 2005-2010 Opal Kelly Incorporated
// $Rev$ $Date$
//------------------------------------------------------------------------
`default_nettype none
`timescale 1ns / 1ps

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

`define CReset                5'b00001
`define CSetWireIns           5'b00100
`define CUpdateWireIns        5'b01000
`define CGetWireOutValue      5'b00010
`define CUpdateWireOuts       5'b01000
`define CActivateTriggerIn    5'b00100
`define CUpdateTriggerOuts    5'b10000
`define CIsTriggered          5'b00010
`define CWriteToPipeIn        5'b00100
`define CReadFromPipeOut      5'b00010
`define CWriteToBTPipeIn      5'b00100
`define CReadFromBTPipeOut    5'b00010

module okHost(
	input   wire [4:0]   okUH,
	output  wire [2:0]   okHU,
	inout   wire [31:0]  okUHU,
	inout   wire         okAA,
	output  wire         okClk,
	output  wire [112:0] okHE,
	input   wire [64:0]  okEH
);

`include "parameters.v"
`include "mappings.v"

// Local okHost signals
wire         hi_clk;
wire         hi_drive;
wire [31:0]  hi_datain;
reg  [31:0]  hi_dataout;
wire [2:0]   hi_cmd;
reg          hi_busy;
reg  [4:0]   ep_command;
reg          ep_blockstrobe;
reg  [7:0]   ep_addr;
wire [31:0]  ep_datain;
reg  [31:0]  ep_dataout;
wire         ep_ready;
reg  [31:0]  reg_addr;
reg          reg_write;
reg  [31:0]  reg_write_data;
reg          reg_read;
wire [31:0]  reg_read_data;


integer     i, j, k;
reg [7:0]   ep;
reg [31:0]  pipeLength;
integer     BlockDelayStates;
integer     blockSize;
integer     blockNum;
integer     registerNum;
integer     ReadyCheckDelay;
integer     PostReadyDelay;

initial begin
	hi_busy = 1'b0;
	hi_dataout = 0;
	i = 0;
	j = 0;
	k = 0;
	ep = 8'h00;
	pipeLength = 0;
	BlockDelayStates = 0;
	blockSize = 0;
	blockNum = 0;
	registerNum = 0;
	ReadyCheckDelay = 0;
	PostReadyDelay = 0;
end

// Mapping of okHost interface to local okHostCall signals 
assign hi_clk    = okUH[0];
assign hi_drive  = okUH[1];
assign hi_cmd    = okUH[4:2];
assign hi_datain = okUHU;
assign okHU[0]   = hi_busy;
assign okUHU     = ~hi_drive ? hi_dataout : 32'hzzzz;
assign okClk     = hi_clk;

// Endpoint interface mapping
assign okHE[okHE_CLK]                              = hi_clk; 
assign okHE[okHE_RESET]                            = ep_command[0]; 
assign okHE[okHE_READ]                             = ep_command[1]; 
assign okHE[okHE_WRITE]                            = ep_command[2]; 
assign okHE[okHE_WIREUPDATE]                       = ep_command[3]; 
assign okHE[okHE_TRIGUPDATE]                       = ep_command[4]; 
assign okHE[okHE_BLOCKSTROBE]                      = ep_blockstrobe; 
assign okHE[okHE_ADDRH:okHE_ADDRL]                 = ep_addr; 
assign okHE[okHE_DATAH:okHE_DATAL]                 = ep_dataout;
assign okHE[okHE_REGADDRH:okHE_REGADDRL]           = reg_addr;
assign okHE[okHE_REGWRITE]                         = reg_write; 
assign okHE[okHE_REGWRITEDATAH:okHE_REGWRITEDATAL] = reg_write_data;
assign okHE[okHE_REGREAD]                          = reg_read; 

assign ep_datain     = okEH[okEH_DATAH:okEH_DATAL];
assign ep_ready      = okEH[okEH_READY];
assign reg_read_data = okEH[okEH_REGREADDATAH:okEH_REGREADDATAL];

always begin

	wait (hi_cmd != `DNOP);
	@(negedge hi_clk) 
	
	case (hi_cmd)
		`DReset: begin
			hi_busy = 1;
			@(negedge hi_clk) 
			ep_blockstrobe = 0; 
			ep_command = `CReset;
			ep_addr = 8'h00;
			ep_dataout = 32'h0000;
			reg_addr = 32'h0000;
			reg_write = 1'b0;
			reg_write_data = 32'h0000;
			reg_read = 1'b0;
			@(negedge hi_clk) ep_command = 5'h00;
			@(negedge hi_clk) hi_busy = 0;
		end

		`DWires: begin
			@(negedge hi_clk)
			case(hi_cmd)
				
				`DUpdateWireIns: begin
					@(negedge hi_clk) hi_busy = 1;
					for (i=0; i<32; i=i+1) begin
						@(negedge hi_clk) ep_command = `CSetWireIns;
						ep_addr = i;
						ep_dataout = hi_datain;
					end
					@(negedge hi_clk) ep_command = 5'h00; ep_addr = 8'h00;
					ep_dataout = 32'h0000;
					@(negedge hi_clk) ep_command = `CUpdateWireIns;  ep_addr = 8'h00;
					@(negedge hi_clk) ep_command = 5'h00; ep_addr = 8'h00;
					@(negedge hi_clk); @(negedge hi_clk); @(negedge hi_clk) hi_busy = 0;
				end
				
				`DUpdateWireOuts: begin
					@(negedge hi_clk) hi_busy = 1;
					@(negedge hi_clk) ep_command = `CUpdateWireOuts; ep_addr = 8'h00;
					@(negedge hi_clk) ep_command = 5'h00;
					@(negedge hi_clk);
					for (i=0; i<32; i=i+1) begin
						ep_command = `CGetWireOutValue;
						ep_addr = (8'h20+i);
						@(negedge hi_clk) hi_dataout = ep_datain;
					end
					ep_command = 5'h00; ep_addr = 8'h00;
					@(negedge hi_clk) hi_dataout = 32'h0000;
					@(negedge hi_clk); @(negedge hi_clk); @(negedge hi_clk);
					@(negedge hi_clk) hi_busy = 0;		
				end
				
				default: $display("Unsupported host sub-command sent");
			endcase
		end
		

		`DTriggers: begin
			@(negedge hi_clk);
			case(hi_cmd)
			
				`DActivateTriggerIn: begin
					@(negedge hi_clk) hi_busy = 1;
					ep = hi_datain[7:0];
					@(negedge hi_clk) ep_command = `CActivateTriggerIn;
					ep_addr = ep;
					ep_dataout = hi_datain;
					@(negedge hi_clk) ep_command = 5'h00;
					ep_addr = 8'h00;
					ep_dataout = 32'h0000;
					@(negedge hi_clk); @(negedge hi_clk); @(negedge hi_clk);
					@(negedge hi_clk) hi_busy = 0;
				end
				
				`DUpdateTriggerOuts: begin
					@(negedge hi_clk) hi_busy = 1;
					ep_command = `CUpdateTriggerOuts; ep_addr = 8'h00;
					@(negedge hi_clk) ep_command = 5'h00;
					@(negedge hi_clk);
					@(negedge hi_clk);
					@(negedge hi_clk);
					
					for (i=0; i<UPDATE_TO_READOUT_CLOCKS; i=i+1)@(negedge hi_clk);
				
					for (i=0; i<32; i=i+1) begin
						ep_command = `CIsTriggered;
						ep_addr = (8'h60+i);
						@(negedge hi_clk) hi_dataout = ep_datain;
					end
					ep_command = 5'h00; ep_addr = 8'h00;
					@(negedge hi_clk); @(negedge hi_clk); @(negedge hi_clk);
					@(negedge hi_clk) hi_busy = 0;
				end
		
				default: $display("Unsupported host sub-command sent");
			endcase
		end
		
		`DPipes: begin
			@(negedge hi_clk);
			case(hi_cmd)
			
				`DWriteToPipeIn: begin
					@(negedge hi_clk) hi_busy = 1; j = 0;
					ep = hi_datain[7:0];
					BlockDelayStates = hi_datain[15:8];
					@(negedge hi_clk) pipeLength = hi_datain;
					for (i=0; i < pipeLength; i=i+1) begin
						@(negedge hi_clk) ep_command = `CWriteToPipeIn; ep_addr = ep;
						ep_dataout = hi_datain;
						j=j+4;
						if (j == 1024) begin
							for (k=0; k < BlockDelayStates; k=k+1) begin
								@(negedge hi_clk) ep_command = 5'h00; ep_addr = 8'h00;
								ep_dataout = 32'h0000;
							end
							j=0;
						end
					end
					@(negedge hi_clk) ep_command = 5'h00; ep_addr = 8'h00;
					ep_dataout = 32'h0000;
					@(negedge hi_clk); @(negedge hi_clk); @(negedge hi_clk);
					@(negedge hi_clk) hi_busy = 0;
				end
		
				`DReadFromPipeOut: begin
					@(negedge hi_clk) hi_busy = 1; j = 0;
					ep = hi_datain[7:0];
					BlockDelayStates = hi_datain[15:8];
					@(negedge hi_clk) pipeLength = hi_datain;
					for (i=0; i < pipeLength; i=i+1) begin
						ep_command = `CReadFromPipeOut; ep_addr = ep;
						@(negedge hi_clk);
						if (i == (pipeLength-1))
							ep_command = 5'h00;
						hi_dataout = ep_datain;
						j=j+4;
						if (j == 1024) begin
							for (k=0; k < BlockDelayStates; k=k+1) begin
								ep_command = 5'h00; ep_addr = 8'h00;
								ep_dataout = 32'h0000;
								@(negedge hi_clk);
							end
							j=0;
						end
					end
					@(negedge hi_clk); @(negedge hi_clk); @(negedge hi_clk);
					@(negedge hi_clk) hi_busy = 0;
				end
		
				`DWriteToBlockPipeIn: begin
					@(negedge hi_clk) hi_busy = 1;
					ep = hi_datain[7:0];
					BlockDelayStates = hi_datain[15:8];
					@(negedge hi_clk) pipeLength = hi_datain;
					@(negedge hi_clk) blockSize = hi_datain;
					@(negedge hi_clk) ReadyCheckDelay = hi_datain[7:0]; PostReadyDelay = hi_datain[15:8];
					ep_addr = ep;
					blockNum = pipeLength/blockSize;
					for (i=0; i<blockNum;i=i+1) begin
						for (j=0; j<ReadyCheckDelay; j=j+1) @(negedge hi_clk);  // Pre ready delay (no checking)
						while (ep_ready !== 1) @(negedge hi_clk);               // Loop while waiting for Ready
						hi_busy = 0;                                            // Act as signal to okHostCalls
						for (j=0; j<PostReadyDelay-1; j=j+1) @(negedge hi_clk); // Post ready asserted delay
						@(negedge hi_clk); hi_busy = 1;
						ep_blockstrobe = 1;                            // Block strobe signal
						@(negedge hi_clk) ep_blockstrobe = 0;          // Turn off block strobe
						@(negedge hi_clk);
						for (j=0; j<blockSize;j=j+1) begin
							@(negedge hi_clk) ep_command = `CWriteToBTPipeIn;
								ep_dataout = hi_datain;
						end
						for (j=0; j<BlockDelayStates; j=j+1) begin
							@(negedge hi_clk) ep_command = 5'h00;
							ep_dataout = 32'h0000;
						end
					end
					
					@(negedge hi_clk) ep_command = 5'h00; ep_addr = 8'h00;
					ep_dataout = 32'h0000;
					@(negedge hi_clk); @(negedge hi_clk); @(negedge hi_clk);
					@(negedge hi_clk) hi_busy = 0; j = 0;
				end
		
				`DReadFromBlockPipeOut: begin
					@(negedge hi_clk) hi_busy = 1;
					ep = hi_datain[7:0];
					BlockDelayStates = hi_datain[15:8];
					@(negedge hi_clk) pipeLength = hi_datain;
					@(negedge hi_clk) blockSize = hi_datain;
					@(negedge hi_clk) ReadyCheckDelay = hi_datain[7:0]; PostReadyDelay = hi_datain[15:8];
					ep_addr = ep;
					blockNum = pipeLength/blockSize;
					for (i=0; i < blockNum; i=i+1) begin
						for (j=0; j<ReadyCheckDelay; j=j+1) @(negedge hi_clk);  // Pre ready delay (no checking)
						while (ep_ready !== 1) @(negedge hi_clk);               // Loop while waiting for Ready
						hi_busy = 0;                                            // Act as signal to okHostCalls
						for (j=0; j<PostReadyDelay-1; j=j+1) @(negedge hi_clk); // Post ready asserted delay
						@(negedge hi_clk); hi_busy = 1;
						ep_blockstrobe = 1;                            // Block strobe signal
						@(negedge hi_clk) ep_blockstrobe = 0;          // Turn off block strobe
						for (j=0; j<blockSize;j=j+1) begin
							ep_command = `CReadFromPipeOut;
							@(negedge hi_clk);
							if (i == (pipeLength-1)) ep_command = 5'h00;
							hi_dataout = ep_datain;
						end
						for (j=0; j < BlockDelayStates; j=j+1) begin
							ep_command = 5'h00; 
							@(negedge hi_clk) hi_dataout = 32'h0000;
						end
					end
					@(negedge hi_clk) ep_command = 5'h00; ep_addr = 8'h00;
					ep_dataout = 32'h0000;
					@(negedge hi_clk); @(negedge hi_clk); @(negedge hi_clk);
					@(negedge hi_clk) hi_busy = 0;
				end
		
				default: $display("Unsupported host sub-command sent");
			endcase
		end
		
		`DRegisters: begin
			@(negedge hi_clk);
			case(hi_cmd)
				
				`DWriteRegister: begin
					@(negedge hi_clk) hi_busy = 1;
					@(negedge hi_clk);
					reg_addr = hi_datain;
					@(negedge hi_clk) 
					reg_write_data = hi_datain;
					@(negedge hi_clk) reg_write = 1'b1;
					@(negedge hi_clk) reg_write = 1'b0;
					@(negedge hi_clk) hi_busy = 0;
				end
				
				`DReadRegister: begin
					@(negedge hi_clk) hi_busy = 1;
					@(negedge hi_clk);
					reg_addr = hi_datain;
					@(negedge hi_clk); 
					reg_read = 1'b1;
					@(negedge hi_clk); 
					hi_dataout = reg_read_data ;
					reg_read = 1'b0;
					@(negedge hi_clk) hi_dataout = 32'h0000;
					@(negedge hi_clk) hi_busy = 0;
				end
				
				`DWriteRegisterSet: begin
					@(negedge hi_clk) hi_busy = 1;
					@(negedge hi_clk); registerNum = hi_datain;
					for (i=0; i < registerNum; i=i+1) begin
						@(negedge hi_clk);
						reg_addr = hi_datain;
						@(negedge hi_clk) 
						reg_write_data = hi_datain;
						@(negedge hi_clk) reg_write = 1'b1;
						@(negedge hi_clk) reg_write = 1'b0;
					end
					@(negedge hi_clk) hi_busy = 0;
				end
				
				`DReadRegisterSet: begin
					@(negedge hi_clk) hi_busy = 1;
					@(negedge hi_clk); registerNum = hi_datain;
					for (i=0; i < registerNum; i=i+1) begin
						@(negedge hi_clk);
						reg_addr = hi_datain;
						@(negedge hi_clk); 
						reg_read = 1'b1;
						@(negedge hi_clk); 
						hi_dataout = reg_read_data ;
						reg_read = 1'b0;
						@(negedge hi_clk); 
					end
						@(negedge hi_clk) hi_dataout = 32'h0000;
					@(negedge hi_clk) hi_busy = 0;
				end
				
				default: $display("Unsupported host sub-command sent");
			endcase
		end
		
		default: $display("Unsupported host command group sent");
	endcase
end

endmodule
