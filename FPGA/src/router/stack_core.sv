//Core for FPGA in stack
`include "BrainDrizzle.sv"
`include "BZ_deserializer.sv"
`include "BZ_serializer.sv"
`include "../core/Core.sv"
`include "tail_bit_reset.sv"
`include "../BD/BDIfc.sv"
`ifdef SIMULATION
	`include "../../quartus/stack_BDIO_PLL.v"
`endif



module stack_core(
input osc,//clock from oscillator on board
// BD ifc
output logic        BD_out_clk_ifc,
input               BD_out_ready,
output logic        BD_out_valid,
output logic [20:0] BD_out_data,
output logic        BD_in_clk_ifc,
output logic        BD_in_ready,
input               _BD_in_valid,
input [33:0]        BD_in_data,
output logic        pReset,
output logic        sReset,
input               adc0,
input               adc1,
//BrainDrizzle Router Signals
input top_in_clk,
input bot_in_clk,
input top_valid_in,
input bot_valid_in,
input top_ready_in,
input bot_ready_in,
input [10:0] top_in,
input [10:0] bot_in,
output [10:0] top_out,
output [10:0] bot_out,
output top_valid_out,
output bot_valid_out,
output top_ready_out,
output bot_ready_out,
output top_out_clk,
output bot_out_clk
);

//RESET TEST
//reg [1:0] test;
//always @ (posedge router_clk or posedge reset) begin
//	if (reset==1)
//		test = 2'b10;
//	else
//		test = 2'b01;
//end

//assign top_valid_out=test[0];

wire BD_in_clk;
wire BD_valid_in;
wire BD_ready_in;
wire [10:0] BD_in;
wire [10:0] BD_out;
wire BD_valid_out;
wire BD_out_clk;
wire BD_ready_out;

//Signals for reset
wire reset;
wire [10:0] top_out_reset;
wire [10:0] top_out_router;
wire valid_reset;
wire top_valid_out_router;

assign top_valid_out = top_valid_out_router & valid_reset;
assign top_out = top_out_router | top_out_reset;

logic BD_in_valid;
assign BD_in_valid = ~_BD_in_valid;

localparam NPCcode = 7;
localparam NPCdata = 20;
localparam NPCroute= 8;
localparam NPCinout = NPCcode + NPCdata + NPCroute;
localparam logic [NPCcode-1:0] NOPcode = 64; // upstream nop code

localparam NBDin = 21;
localparam NBDout = 34;


wire BD_in_clk_int; // clocks to BD's handshakers
wire BD_out_clk_int;
// Clocks generated by PLL
wire base_BD_clk;
wire skewed_BD_clk;
wire router_clk;
wire core_clk;
//
assign BD_in_clk_int=base_BD_clk;
assign BD_out_clk_int=base_BD_clk;
assign BD_in_clk_ifc=skewed_BD_clk;
assign BD_out_clk_ifc=skewed_BD_clk;

// channels between OK ifc and core design
Channel #(NPCinout) PC_downstream();
Channel #(NPCinout + NPCroute) PC_upstream();

// channels between core design and BD ifc
Channel #(NBDin) BD_downstream();
Channel #(NBDout) BD_upstream();

wire req_0, req_1;
BrainDrizzle router_node (
 .clk				(router_clk),
 .reset			(reset),
 .top_in_clk	(top_in_clk),
 .bot_in_clk	(bot_in_clk),
 .BD_in_clk		(router_clk),
 .top_valid_in	(top_valid_in),
 .bot_valid_in	(bot_valid_in),
 .BD_valid_in	(BD_valid_in),//CHANGE BACK
 .top_ready_in	(top_ready_in),// CHANGE THIS BACK TO top_ready_in),
 .BD_ready_in	(BD_ready_in),
 .bot_ready_in	(bot_ready_in),
 .top_in			(top_in),//CHANGE BACK
 .bot_in			(bot_in),
 .BD_in			(BD_in),//CHANGE BACK
 .top_out		(top_out_router),
 .bot_out		(bot_out),
 .BD_out			(BD_out),
 .top_valid_out(top_valid_out_router),
 .bot_valid_out(bot_valid_out),
 .BD_valid_out	(BD_valid_out),
 .top_ready_out(top_ready_out),
 .BD_ready_out	(BD_ready_out),
 .bot_ready_out(bot_ready_out),
 .top_out_clk	(),
 .BD_out_clk	(BD_out_clk),
 .bot_out_clk	(),
 .sent_something_to_top	(req_0),
 .sent_something_to_BD	(req_1)
);

//module tail_bit_reset(
//	input below_clk, //clock from the board below us
//	input our_clk, //this board's clock
//	input tail_bit, //the value of the tail bit pin
//	output reg reset, //reset signal for this board
//	output reg next_tail_bit //the tail bit to send to the next board
//	);
tail_bit_reset gen_reset(
	.below_clk		(bot_in_clk),
	.our_clk			(router_clk),
	.bot_in		(bot_in),
	.bot_valid_in (bot_valid_in),
	.reset			(reset),
	.top_out_reset(top_out_reset),
	.valid_reset  (valid_reset)
	);

// channels between serdes and core design
Channel #(NPCinout) Des_out();
Channel #(NPCinout) Ser_in();

// assign Des_out.a = Ser_in.a;
// assign Ser_in.v = Des_out.v;
// assign Ser_in.d = {5'b10000, Des_out.d[26:0]};

BZ_deserializer deserializer (
	.PC_out_channel(Des_out), //output channel for the Core
	.isempty(~BD_valid_out), //isempty signal for the fifo feeding us packets
	.data_in(BD_out), //data from the fifo
	.rdreq(BD_ready_in), //read request for fifo
	.clk(BD_out_clk),
	.reset(reset)
);
BZ_serializer serializer (
	.PC_in_channel(Ser_in), //channel from the BD that has data for us
	.is_full(~BD_ready_out), //full signal for the fifo this places stuff into
	.data_out(BD_in), //data to write to fifo
	.wrreq(BD_valid_in), //fifo write request
	.clk(router_clk),
	.reset(reset)
);

DCChannelFIFO32 input_channel_fifo(Des_out, PC_downstream, router_clk, core_clk, reset);
DCChannelFIFO42 output_channel_fifo(PC_upstream, Ser_in, core_clk, router_clk, reset);

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
  .clk(core_clk),
  .reset(reset)
  );

//ADD NEW BDCLKGEN WHICH TAKES IN 50 MHz and generates 200 MHz, 100MHz, and 2 BD clks
stack_BDIO_PLL stack_clockgen(
	.inclk0	(osc),
	.c0		(base_BD_clk),
	.c1		(skewed_BD_clk),
	.c2		(router_clk),
	.c3		(core_clk),
	.c4    (transmit_clk)
	);

assign top_out_clk = transmit_clk;
assign bot_out_clk = transmit_clk;

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
  .core_clk(core_clk),
  .BD_in_clk_int(BD_in_clk_int),
  .BD_out_clk_int(BD_out_clk_int),
  .reset(reset)//NEW RESET STUFF
);

endmodule