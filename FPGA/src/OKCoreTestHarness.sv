`include "OKIfc.sv"
`include "CoreTestHarness.sv"

module OKCoreTestHarness (
	input  wire [4:0]   okUH,
	output wire [2:0]   okHU,
	inout  wire [31:0]  okUHU,
	inout  wire         okAA,

  input wire user_reset,
	output wire [3:0]   led
	);

localparam NPCcode = 8;
localparam NPCdata = 24;
localparam NPCinout = NPCcode + NPCdata;
localparam logic [NPCcode-1:0] NOPcode = 64; // upstream nop code
localparam PCtoBDcode = {NPCcode{1'b1}}; // downstream traffic with this code goes to the BDInput
localparam BDtoPCcode = {NPCcode{1'b1}}; // upstream traffic with this code came from BD

wire okClk; // OKHost has a PLL inside it, generates 100MHz clock for the rest of the design

// channels between OK ifc and core design
Channel #(NPCinout) PC_downstream();
Channel #(NPCinout) PC_upstream();

// Opal-Kelly HDL host and endpoints
OKIfc #(
  NPCcode, 
  NOPcode) 
ok_ifc(
	.okUH(okUH),
	.okHU(okHU),
	.okUHU(okUHU),
	.okAA(okAA),
	.led(led),
  .okClk(okClk),
  .user_reset(user_reset),
  .PC_downstream(PC_downstream),
  .PC_upstream(PC_upstream));

// core design with loopbacks around BD-facing connections
CoreTestHarness #(
  NPCcode,
  PCtoBDcode,
  BDtoPCcode) 
core_harness(
  .PC_in(PC_downstream),
  .PC_out(PC_upstream),
  .clk(okClk),
  .reset(user_reset));

endmodule
