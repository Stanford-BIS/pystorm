//------------------------------------------------------------------------
// okRegisterBridge
//
// This module simulates the "Register Bridge" endpoint.
//
//------------------------------------------------------------------------
// Copyright (c) 2004-2011 Opal Kelly Incorporated
// $Id$
//------------------------------------------------------------------------
`default_nettype none
`timescale 1ns / 1ps

module okRegisterBridge(
	input  wire [112:0] okHE,
	output wire [64:0]  okEH,
	output wire [31:0]  ep_address,
	output wire         ep_write,
	output wire [31:0]  ep_dataout,
	output wire         ep_read,
	input  wire [31:0]  ep_datain
	);

`include "mappings.v"

assign ep_address = ti_reg_addr;
assign ep_write   = ti_reg_write;
assign ep_dataout = ti_reg_write_data;
assign ep_read    = ti_reg_read;

assign okEH[okEH_REGREADDATAH:okEH_REGREADDATAL] = ep_datain;
assign okEH[okEH_READY] = 1'b0;
assign okEH[okEH_DATAH:okEH_DATAL] = 32'h0;

endmodule