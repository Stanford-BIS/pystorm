module OKCoreTestHarness (
  // OK ifc
	input  wire [4:0]   okUH,
	output wire [2:0]   okHU,
	inout  wire [31:0]  okUHU,
	inout  wire         okAA,

  input wire clkp,
  input wire clkn,
  input wire user_reset,
	output wire [3:0]   led

  // BD ifc
  output logic        BD_out_clk,
  input               BD_out_ready,
  output logic        BD_out_valid,
  output logic [20:0] BD_out_data,

  output logic        BD_in_clk,
  output logic        BD_in_ready,
  input               BD_in_valid,
  input [33:0]        BD_in_data,
  
  output logic pReset,
  output logic sReset,

  input adc0,
  input adc1
	);

localparam NPCcode = 8;
localparam NPCdata = 24;
localparam NPCinout = NPCcode + NPCdata;
localparam logic [NPCcode-1:0] NOPcode = 64; // upstream nop code

localparam NBDin = 21;
localparam NBDout = 34;

wire okClk; // OKHost has a PLL inside it, generates 100MHz clock for the rest of the design

// channels between OK ifc and core design
Channel #(NPCinout) PC_downstream();
Channel #(NPCinout) PC_upstream();

Channel #(NBDin) BD_downstream();
Channel #(NBDout) BD_upstream();


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
Core core(
  .PC_out(PC_upstream)
  .PC_in(PC_downstream)
  .BD_out(BD_downstream)
  .BD_in(BD_upstream)
  .clk(okClk),
  .reset(user_reset));

FPGA_TO_BD bd_down_ifc(
  .bd_channel(BD_downstream),
  .valid(BD_out_valid),
  .data(BD_out_data),
  .ready(BD_out_ready),
  .reset(user_reset),
  .clk(okClk));

BD_TO_FPGA bd_up_ifc(
  .bd_channel(BD_upstream),
  .valid(BD_in_valid),
  .data(BD_in_data),
  .ready(BD_in_ready),
  .reset(user_reset),
  .clk(okClk));


endmodule
