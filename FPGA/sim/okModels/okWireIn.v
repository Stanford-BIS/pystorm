//------------------------------------------------------------------------
// okWireIn.v
//
// This module simulates the "Wire In" endpoint.
//
//------------------------------------------------------------------------
// Copyright (c) 2005-2010 Opal Kelly Incorporated
// $Rev$ $Date$
//------------------------------------------------------------------------
`default_nettype none
`timescale 1ns / 1ps

module okWireIn(
	input  wire [112:0] okHE,
	input  wire [7:0]   ep_addr,
	output reg  [31:0]  ep_dataout
	);

`include "parameters.v" 
`include "mappings.v"

reg  [31:0] ep_datahold;

always @(posedge ti_clk) begin
	if ((ti_write == 1'b1) && (ti_addr == ep_addr)) ep_datahold = ti_datain;
	if (ti_wireupdate == 1'b1) ep_dataout = #TDOUT_DELAY ep_datahold;
	if (ti_reset == 1'b1) begin
		ep_datahold = #TDOUT_DELAY 0;
		ep_dataout  = 0;
	end
end
	

endmodule
