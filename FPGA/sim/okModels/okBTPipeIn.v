//------------------------------------------------------------------------
// okBTPipeIn.v
//
// This module simulates the "Block Input Pipe" endpoint.
//
//------------------------------------------------------------------------
// Copyright (c) 2005-2010 Opal Kelly Incorporated
// $Rev$ $Date$
//------------------------------------------------------------------------
`default_nettype none
`timescale 1ns / 1ps

module okBTPipeIn(
	input  wire [112:0] okHE,
	output wire [64:0]  okEH,
	input  wire [7:0]   ep_addr,
	output reg          ep_write,
	output wire         ep_blockstrobe,
	output reg  [31:0]  ep_dataout,
	input  wire         ep_ready
	);

`include "parameters.v" 
`include "mappings.v"

assign ep_blockstrobe              = ti_blockstrobe & (ti_addr == ep_addr);
assign okEH[okEH_DATAH:okEH_DATAL] = 0;
assign okEH[okEH_READY]            = (ti_addr == ep_addr) ? (ep_ready) : (0);
assign okEH[okEH_REGREADDATAH:okEH_REGREADDATAL] = 32'b0;

always @(posedge ti_clk) begin
	#TDOUT_DELAY;
	ep_write = 0;
	if (ti_reset == 1) begin
		ep_dataout  = 0;
	end else if ((ti_write == 1) && (ti_addr == ep_addr)) begin
		ep_dataout = ti_datain;
		ep_write = 1;
	end
end

endmodule
