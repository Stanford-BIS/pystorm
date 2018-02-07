`include "lib/Channel.svh"
`include "core/Core.sv"
`include "OK/OKIfc.sv"
`include "BD/BDIfc.sv"
`include "BD/BDClkGen.sv"
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
  input               _BD_in_valid,
  input [33:0]        BD_in_data,
  
  output logic        pReset,
  output logic        sReset,

  input               adc0,
  input               adc1
	);


logic BD_in_valid;
assign BD_in_valid = ~_BD_in_valid;

localparam NPCcode = 7;
localparam NPCdata = 20;
localparam NPCroute = 5;

localparam NOKinout = NPCcode + NPCdata + NPCroute; 

localparam NPCin = NPCcode + NPCdata; // discard route
localparam NPCout = NPCcode + NPCdata + NPCroute;

localparam NBDin = 21;
localparam NBDout = 34;

//GO_HOME route
localparam logic [NPCroute-1:0] GO_HOME_rt = 31;

// internal clocks
wire okClk; // OKHost has a PLL inside it, generates 100MHz clock for the rest of the design
wire sys_clk; // to PLL input
wire BD_in_clk_int; // clocks to BD's handshakers
wire BD_out_clk_int;

// XXX PC/OK isn't a great name anymore
// OK refers to the data on the BD-facing side of the OK ifc
// there's a transformation to/from the PC data with the route
// PC refers to the data on the PC-facing side of the core

// channels between OK ifc and core design
Channel #(NOKinout) OK_downstream();
Channel #(NOKinout) OK_upstream();

Channel #(NPCin) PC_downstream();
Channel #(NPCout) PC_upstream();

// convert with route
logic [NPCroute-1:0] route_down;
logic [NPCcode-1:0] code_down;
logic [NPCdata-1:0] data_down;

logic [NPCroute-1:0] route_up;
logic [NPCcode-1:0] code_up;
logic [NPCdata-1:0] data_up;

// discard route coming out of OKIfc, going to core
assign {route_down, code_down, data_down} = OK_downstream.d;
assign PC_downstream.v = OK_downstream.v;
assign PC_downstream.d = {code_down, data_down};
assign OK_downstream.a = PC_downstream.v;

// discard route (set to GO_HOME_rt) coming out of core, going to OKIfc
assign {code_up, data_up} = PC_upstream.d;
assign OK_upstream.v = PC_upstream.v;
assign OK_upstream.d = {GO_HOME_rt, code_up, data_up};
assign PC_upstream.a = OK_upstream.v;

// channels between core design and BD ifc
Channel #(NBDin) BD_downstream();
Channel #(NBDout) BD_upstream();

// led control. Flashes for handshakes.
logic [3:0] led_in;
assign led_in[0] = PC_downstream.v;
assign led_in[1] = BD_out_valid;
assign led_in[2] = BD_in_valid;
assign led_in[3] = PC_upstream.v;

// Opal-Kelly HDL host and endpoints, with FIFOs
OKIfc #(
  NPCroute,
  NPCcode,
  NPCdata,
  GO_HOME_rt)
ok_ifc(
	.okUH(okUH),
	.okHU(okHU),
	.okUHU(okUHU),
	.okAA(okAA),
	.led(led),
	.led_in(led_in),
  .okClk(okClk),
  .user_reset(user_reset),
  .PC_downstream(OK_downstream),
  .PC_upstream(OK_upstream));

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

// get single-ended clock
SysClkBuf sys_clk_buf(.datain(sys_clk_p), .datain_b(sys_clk_n), .dataout(sys_clk));

logic pll_locked; // unused
logic reset_BDIO;
assign reset_BDIO = user_reset;

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
  .BD_in_clk_ext(BD_in_clk),
  .BD_out_clk_ext(BD_out_clk),
  .reset(reset_BDIO));

endmodule
