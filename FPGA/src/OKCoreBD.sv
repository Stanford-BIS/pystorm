`include "Channel.svh"
`include "Core.sv"
`include "OKIfc.sv"
`include "BDIfc.sv"
`include "BDClkGen.sv"
// for quartus, we add external IP to the project
`ifdef SIMULATION
  `include "../quartus/SysClkBuf.v"
`endif

module OKCoreBD (
  // OK ifc
	input  wire [4:0]   okUH,
	output wire [2:0]   okHU,
	inout  wire [31:0]  okUHU,
	inout  wire         okAA,

  input wire          sys_clk_p,
  input wire          sys_clk_n,
  input wire          user_reset,
	output wire [3:0]   led,

  // BD ifc
  output logic        BD_out_clk,
  input               BD_out_ready,
  output logic        BD_out_valid,
  output logic [20:0] BD_out_data,

  output logic        BD_in_clk,
  output logic        BD_in_ready,
  input               BD_in_valid,
  input [33:0]        BD_in_data,
  
  output logic        pReset,
  output logic        sReset,

  input               adc0,
  input               adc1
	);

localparam NPCcode = 8;
localparam NPCdata = 24;
localparam NPCinout = NPCcode + NPCdata;
localparam logic [NPCcode-1:0] NOPcode = 64; // upstream nop code

localparam NBDin = 21;
localparam NBDout = 34;

// internal clocks
wire okClk; // OKHost has a PLL inside it, generates 100MHz clock for the rest of the design
wire sys_clk; // to PLL input
wire BD_in_clk_int; // clocks to BD's handshakers
wire BD_out_clk_int;

// channels between OK ifc and core design
Channel #(NPCinout) PC_downstream();
Channel #(NPCinout) PC_upstream();

// channels between core design and BD ifc
Channel #(NBDin) BD_downstream();
Channel #(NBDout) BD_upstream();


// Opal-Kelly HDL host and endpoints, with FIFOs
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

// core design
Core core(
  .PC_out(PC_upstream),
  .PC_in(PC_downstream),
  .BD_out(BD_downstream),
  .BD_in(BD_upstream),
  .pReset(pReset),
  .sReset(sReset),
  .adc0(adc0),
  .adc1(adc1),
  .clk(okClk),
  .reset(user_reset));

// this is a low-level altera primitive, to turn LVDS -> single-ended, supposedly. 
// It's what OK uses in the Counters example to get their clk
SysClkBuf sys_clk_buf(.datain(sys_clk_p), .datain_b(sys_clk_n), .dataout(sys_clk));

logic pll_locked;
logic reset_BDIO;
assign reset_BDIO = ~pll_locked | user_reset;

// PLL generates BD_IO_clk
BDClkGen bd_clk_gen(
  .BD_in_clk_int(BD_in_clk_int),
  .BD_in_clk_ext(BD_in_clk),
  .BD_out_clk_int(BD_out_clk_int),
  .BD_out_clk_ext(BD_out_clk),
  .pll_locked(pll_locked),
  .clk(sys_clk), 
  .reset(user_reset));


// BD handshakers and FIFOs
BDIfc BD_ifc(
  .core_up(BD_upstream),
  .core_dn(BD_downstream),
  .BD_out_ready(BD_out_ready),
  .BD_out_valid(BD_out_valid),
  .BD_out_data(BD_out_data),
  .BD_in_ready(BD_in_ready),
  .BD_in_valid(BD_in_valid),
  .BD_in_data(BD_in_data),
  .core_clk(okClk),
  .BD_in_clk_int(BD_in_clk_int),
  .BD_out_clk_int(BD_out_clk_int),
  .reset(reset_BDIO));

endmodule
