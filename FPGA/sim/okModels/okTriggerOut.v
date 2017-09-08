//------------------------------------------------------------------------
// okTriggerOut.v
//
// This module simulates the "Trigger Out" endpoint.
//
//------------------------------------------------------------------------
// Copyright (c) 2005-2010 Opal Kelly Incorporated
// $Rev$ $Date$
//------------------------------------------------------------------------
`default_nettype none
`timescale 1ns / 1ps

module okTriggerOut(
	input  wire [112:0] okHE,
	output wire [64:0]  okEH,
	input  wire [7:0]   ep_addr,
	input  wire         ep_clk,
	input  wire [31:0]  ep_trigger
	);

`include "parameters.v" 
`include "mappings.v"

reg  [31:0] eptrig;
reg  [31:0] ep_trigger_p1;
reg  [31:0] trighold;
reg         captrig;

assign okEH[okEH_DATAH:okEH_DATAL] = (ti_addr == ep_addr) ? (trighold) : (0);
assign okEH[okEH_READY]            = 0;
assign okEH[okEH_REGREADDATAH:okEH_REGREADDATAL] = 32'b0;

always @(posedge ti_clk) if (ti_trigupdate == 1'b1) captrig = 1;

always @(posedge ep_clk or posedge ti_reset) begin
	if (ti_reset == 1) begin
		ep_trigger_p1 = 0;
		trighold = 0;
		eptrig = 0;
		captrig = 0;
	end 
	else begin
		if (captrig == 1) begin
			trighold = eptrig;
			eptrig = ep_trigger;
			captrig = 0;   
		end
		else eptrig = eptrig | (ep_trigger & ~ep_trigger_p1);
		ep_trigger_p1 = ep_trigger;
  end
end

endmodule
