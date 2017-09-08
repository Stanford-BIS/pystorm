//------------------------------------------------------------------------
// First.v
//
// A simple example for getting started with FrontPanel.  This sample
// connects the on-board buttons to Wire Outs and the on-board LEDs
// to Wire Ins so that FrontPanel can observe the buttons and control
// the LEDs.
//
// tabstop: 3
//
// Copyright (c) 2005-2015
// Opal Kelly Incorporated
//------------------------------------------------------------------------

`default_nettype none
`timescale 1ns / 1ps

module First(
	input  wire [4:0]   okUH,
	output wire [2:0]   okHU,
	inout  wire [31:0]  okUHU,
	inout  wire         okAA,

	output wire [7:0]   led
	);

// Target interface bus:
wire         okClk;
wire [112:0] okHE;
wire [64:0]  okEH;

// Endpoint connections:
wire [31:0]  ep00wire;
wire [31:0]  ep01wire;
wire [31:0]  ep02wire;
wire [31:0]  ep20wire;
reg  [31:0]  ep21wire;

assign led      = ~ep00wire[7:0];
assign ep20wire = {32'h0000};
always @(posedge okClk)  ep21wire <= ep01wire + ep02wire;

// Instantiate the okHost and connect endpoints.
wire [65*2-1:0]  okEHx;
okHost okHI(
	.okUH(okUH),
	.okHU(okHU),
	.okUHU(okUHU),
	.okAA(okAA),
	.okClk(okClk),
	.okHE(okHE), 
	.okEH(okEH)
);

okWireOR # (.N(2)) wireOR (okEH, okEHx);

okWireIn     ep00 (.okHE(okHE),                             .ep_addr(8'h00), .ep_dataout(ep00wire));
okWireIn     ep01 (.okHE(okHE),                             .ep_addr(8'h01), .ep_dataout(ep01wire));
okWireIn     ep02 (.okHE(okHE),                             .ep_addr(8'h02), .ep_dataout(ep02wire));
okWireOut    ep20 (.okHE(okHE), .okEH(okEHx[ 0*65 +: 65 ]), .ep_addr(8'h20), .ep_datain(ep20wire));
okWireOut    ep21 (.okHE(okHE), .okEH(okEHx[ 1*65 +: 65 ]), .ep_addr(8'h21), .ep_datain(ep21wire));

endmodule
