//Core for BrainDrizzle host FPGA
`include "BrainDrizzle.sv"
`include	"BZ_deserializer.sv"
`include "BZ_serializer.sv"
`include "../OK/OKIfc.sv"

// for quartus, we add external IP to the project
`ifdef SIMULATION
	`include "../../quartus/SysClkBuf.v"
	`include "../../quartus/BZ_host_core_PLL.v"
`endif


module BZ_host_core (
//OK Ifc signals
input  wire [4:0]   okUH,
output wire [2:0]   okHU,
inout  wire [31:0]  okUHU,
inout  wire         okAA,
input wire          sys_clk_p,
input wire          sys_clk_n,
input wire          user_reset,
output wire [3:0]   led,
//BrainDrizzle Router Signals
input top_in_clk,
input top_valid_in,
input top_ready_in,
input [10:0] top_in,
output [10:0] top_out,
output top_valid_out,
output top_ready_out,
output top_out_clk,
);

wire router_clk;
wire bot_valid_in;
wire bot_ready_in;
wire [10:0] bot_in;
wire [10:0] bot_out;
wire bot_valid_out;
wire bot_out_clk;


localparam NPCcode = 8;
localparam NPCdata = 24;
localparam NPCinout = NPCcode + NPCdata;
localparam logic [NPCcode-1:0] NOPcode = 64; // upstream nop code

// internal clocks
wire okClk; // OKHost has a PLL inside it, generates 100MHz clock for the rest of the design
wire sys_clk; // 100 MHz clock, send to PLL to generate 200MHz clock for router node

// channels between OK ifc and core design
Channel #(NPCinout) PC_downstream();
Channel #(NPCinout) PC_upstream();

// get single-ended clock
SysClkBuf sys_clk_buf(.datain(sys_clk_p), .datain_b(sys_clk_n), .dataout(sys_clk));

//Generate 200 MHz Clock for router node
BZ_host_core_PLL BZ_host_PLL(
	.refclk(sys_clk),
	.rst(user_reset),
	.outclk_0(router_clk),
	.locked()
	);

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
	.led_in(led_in),
	.okClk(okClk),
	.user_reset(user_reset),
	.PC_downstream(PC_downstream),
	.PC_upstream(PC_upstream)
 );



BrainDrizzle router_node (
 .clk				(router_clk),
 .reset			(reset),
 .top_in_clk	(top_in_clk),
 .bot_in_clk	(router_clk),
 .BD_in_clk		(0),//No BD
 .top_valid_in	(top_valid_in),
 .bot_valid_in	(bot_valid_in),
 .BD_valid_in	(0),//No BD
 .top_ready_in	(top_ready_in),
 .BD_ready_in	(0),//No BD
 .bot_ready_in	(bot_ready_in),
 .top_in			(top_in),
 .bot_in			(bot_in),
 .BD_in			(0),//No BD
 .top_out		(top_out),
 .bot_out		(bot_out),
 .BD_out			(),//No BD
 .top_valid_out(top_valid_out),
 .bot_valid_out(bot_valid_out),
 .BD_valid_out	(),//No BD
 .top_ready_out(top_ready_out),
 .BD_ready_out	(),//No BD
 .bot_ready_out(bot_ready_out),
 .top_out_clk	(top_out_clk),
 .BD_out_clk	(),//No BD
 .bot_out_clk	(bot_out_clk)
);

BZ_deserializer deserializer (
	.PC_out_channel(PC_upstream), //output channel for the Core
	.isempty(~bot_valid_out), //isempty signal for the fifo feeding us packets ???????
	.data_in(bot_out), //data from the fifo
	.rdreq(bot_ready_in), //read request for fifo
	.clk(bot_out_clk),
	.reset(reset)
);
BZ_serializer serializer (
	.PC_in_channel(PC_downstream), //channel from the PC that has data for us
	.is_full(~bot_ready_out), //full signal for the fifo this places stuff into
	.data_out(bot_in), //data to write to fifo
	.wrreq(bot_valid_in), //fifo write request
	.clk(router_clk),
	.reset(reset)
);