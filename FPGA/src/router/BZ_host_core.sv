//Core for BrainDrizzle host FPGA
`include "BrainDrizzle.sv"
`include "BZ_deserializer.sv"
`include "BZ_serializer.sv"
`include "../OK/OKIfc.sv"
`include "../core/Deserializer.sv"

// for quartus, we add external IP to the project
`ifdef SIMULATION
	`include "../../quartus/SysClkBuf.v"
	`include "../../quartus/BZ_host_core_PLL.v"
	`include "../../quartus/BZ_host_core_PLL/BZ_host_core_PLL_0002.v"
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
output top_out_clk
);

wire router_clk;
wire bot_valid_in;
wire bot_ready_in;
wire [10:0] bot_in;
wire [10:0] bot_out;
wire bot_valid_out;
wire bot_out_clk;


localparam NPCcode = 7;
localparam NPCdata =20;
localparam NPCroute = 5;
localparam NPCinout = NPCcode + NPCdata + NPCroute;
localparam logic [NPCcode-1:0] NOPcode = 64; // upstream nop code

// internal clocks
wire okClk; // OKHost has a PLL inside it, generates 100MHz clock for the rest of the design
wire sys_clk; // 100 MHz clock, send to PLL to generate 200MHz clock for router node


//signals for reset
reg r1,r2,r3,r4;
wire tail_out_reset;
wire [10:0] top_out_router;
assign top_out = {tail_out_reset | top_out_router[10], top_out_router[9:0]};
wire bot_ready_out;

// channels between OK ifc and core design
Channel #(NPCinout) PC_downstream();
Channel #(NPCinout) PC_upstream();
// channels between serdes and core design
Channel #(NPCinout) Des_out();
Channel #(NPCinout) Ser_in();

// get single-ended clock
SysClkBuf sys_clk_buf(.datain(sys_clk_p), .datain_b(sys_clk_n), .dataout(sys_clk));

// led control. Flashes for handshakes.
logic [3:0] led_in;
assign led_in[0] = PC_downstream.v;
//assign led_in[1] = 0;
//assign led_in[3] = 0;
assign led_in[2] = PC_upstream.v;
//Generate 200 MHz Clock for router node
wire transmit_clk;
assign top_out_clk = transmit_clk;
BZ_host_core_PLL BZ_host_PLL(
	.refclk(sys_clk),
	.rst(0),
	.outclk_0(router_clk),
	.outclk_1(transmit_clk),
	.locked()
	);

////////////////////////////////////////////
//
// Input from OK
//
// [route | code | payload ]
//	  5      7        20
//
// Input to Router
//
// [ route | data ]
//	  10	  30
//
Channel #(NPCinout) OK_downstream();

assign PC_downstream.v = OK_downstream.v;
assign OK_downstream.a = PC_downstream.a;
assign PC_downstream.d = {5'b0, OK_downstream.d[31:27], 3'b0, OK_downstream.d[26:0]};

////////////////////////////////////////////

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
	.PC_downstream(OK_downstream),
	.PC_upstream(PC_upstream)
 );


wire BD_valid_out;
BrainDrizzle router_node (
 .clk				(router_clk),
 .reset			(user_reset),
 .top_in_clk	(top_in_clk),
 .bot_in_clk	(router_clk),
 .BD_in_clk		(0),//No BD
 .top_valid_in	(top_valid_in),
 .bot_valid_in	(bot_valid_in),
 .BD_valid_in	(0),//No BD
 .top_ready_in	(top_ready_in),
 .BD_ready_in	(1),//No BD. Discard anything coming out of BD_out
 .bot_ready_in	(bot_ready_in),
 .top_in			(top_in),
 .bot_in			(bot_in),
 .BD_in			(0),//No BD
 .top_out		(top_out_router),
 .bot_out		(bot_out),
 .BD_out			(),//No BD
 .top_valid_out(top_valid_out),
 .bot_valid_out(bot_valid_out),
 .BD_valid_out	(BD_valid_out),//No BD
 .top_ready_out(top_ready_out),
 .BD_ready_out	(),//No BD
 .bot_ready_out(bot_ready_out),
 .top_out_clk	(),
 .BD_out_clk	(),//No BD
 .bot_out_clk	(bot_out_clk),
 .sent_something_to_top(),//led_in[3]),
 .sent_something_to_BD()//led_in[1])
);
//assign led_in[1]=bot_ready_out;
reg something_top,something_in=0;
always @ (top_valid_out or top_valid_in or user_reset) begin
	if (user_reset==1) begin
		something_top=0;
		something_in=0;
	end
	else if (user_reset==0) begin
		if (top_valid_out==1)
			something_top=1;
		if (top_valid_in==1)
			something_in=1;
	end	
end
assign led_in[3]=something_top;
assign led_in[1]=something_in;

DCChannelFIFO32 input_channel_fifo(Des_out, PC_upstream, router_clk, okClk, user_reset);
DCChannelFIFO42 output_channel_fifo(PC_downstream, Ser_in, okClk, router_clk, user_reset);


BZ_deserializer deserializer (
	.PC_out_channel(Des_out), //output channel for the Core
	.isempty(~bot_valid_out), //isempty signal for the fifo feeding us packets ???????
	.data_in(bot_out), //data from the fifo
	.rdreq(bot_ready_in), //read request for fifo
	.clk(bot_out_clk),
	.reset(user_reset)
);
BZ_serializer serializer (
	.PC_in_channel(Ser_in), //channel from the PC that has data for us
	.is_full(~bot_ready_out), //full signal for the fifo this places stuff into
	.data_out(bot_in), //data to write to fifo
	.wrreq(bot_valid_in), //fifo write request
	.clk(router_clk),
	.reset(user_reset)
);


//sending reset to boards above
always @ (posedge router_clk) begin
	r1<=user_reset;
	r2<=r1;
	r3<=r2;
	r4<=r3;
end

assign tail_out_reset=r1|r2|r3|r4;

endmodule