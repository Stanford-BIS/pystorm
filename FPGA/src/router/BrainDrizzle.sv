//BrainDrizzle Top
`include "InputController.sv"
`include "allocator.sv"


module BrainDrizzle(
input clk, 
input top_in_clk,
input bot_in_clk,
input BD_in_clk,
input valid_top,
input valid_bot,
input valid_BD,
input [10:0] top_in,
input [10:0] bot_in,
input [10:0] BD_in,
//Will be removed after Zach is finished with inter-board handshaking
input top_out_rd,
input	BD_out_rd,
input bot_out_rd,
input top_in_wr,
input BD_in_wr,
input bot_in_wr,
output top_in_full,
output BD_in_full,
output bot_in_full,
////////////////////////////////
output [10:0] top_out,
output [10:0] bot_out,
output [10:0] BD_out,
output ready_top,
output ready_bot,
output ready_BD,
output top_out_clk,
output BD_out_clk,
output bot_out_clk);

//Signals which originate from allocators
wire top_wr;
wire BD_wr;
wire bot_wr;
wire top_ready_0;
wire top_ready_1;
wire BD_ready_0;
wire BD_ready_1;
wire bot_ready_0;
wire bot_ready_1;
wire [10:0] top_out_to_FIFO;
wire [10:0] BD_out_to_FIFO;
wire [10:0] bot_out_to_FIFO;
//Signals which originate from input controllers
wire top_req_0;
wire top_req_1;
wire bot_req_0;
wire bot_req_1;
wire BD_req_0;
wire BD_req_1;
wire top_read;
wire BD_read;
wire bot_read;
wire [10:0] top_data;
wire [10:0] bot_data;
wire [10:0] BD_data;
//Signals which originate from input FIFOs
wire top_in_FIFO_empty;
wire BD_in_FIFO_empty;
wire bot_in_FIFO_empty;
wire [10:0] top_in_from_FIFO;
wire [10:0] BD_in_from_FIFO;
wire [10:0] bot_in_from_FIFO;


//Signals which originate from output FIFOs
wire top_out_FIFO_full;
wire bot_out_FIFO_full;
wire BD_out_FIFO_full;

//3 output allocators

// allocator (
//input clk,
//input req_0,
//input req_1,
//input out_FIFO_full,
//input [10:0] data_in_0,
//input [10:0] data_in_1,
//output reg ready_0,
//output reg ready_1,
//output reg out_FIFO_wr,
//output reg [10:0] data_out);

allocator top_out_allocator (
clk,
BD_req_1,
bot_req_1,
top_out_FIFO_full,
BD_data,
bot_data,
top_ready_0,
top_ready_1,
top_wr,
top_out_to_FIFO
);

allocator BD_out_allocator (
clk,
bot_req_0,
top_req_0,
BD_out_FIFO_full,
bot_data,
top_data,
BD_ready_0,
BD_ready_1,
BD_wr,
BD_out_to_FIFO
);

allocator bot_out_allocator (
clk,
BD_req_0,
top_req_1,
bot_out_FIFO_full,
BD_data,
top_data,
bot_ready_0,
bot_ready_1,
bot_wr,
bot_out_to_FIFO
);

//3 input controllers

//module InputController (
//input clk,
//input empty,
//input ready_0,
//input ready_1,
//input [10:0] data_in,
//output reg req_0,
//output reg req_1,
//output reg read,
//output reg [10:0] data_out
//);

InputController top_in_controller (
clk,
top_in_FIFO_empty,
BD_ready_1,
bot_ready_1,
top_in_from_FIFO,
top_req_0,
top_req_1,
top_read,
top_data
);

InputController BD_in_controller (
clk,
BD_in_FIFO_empty,
bot_ready_0,
top_ready_0,
BD_in_from_FIFO,
BD_req_0,
BD_req_1,
BD_read,
BD_data
);

InputController bot_in_controller (
clk,
bot_in_FIFO_empty,
BD_ready_0,
top_ready_1,
bot_in_from_FIFO,
bot_req_0,
bot_req_1,
bot_read,
bot_data
);

//3 Input FIFOs
//module routerDCFIFO (
//	data,
//	rdclk,
//	rdreq,
//	wrclk,
//	wrreq,
//	q,
//	rdempty,
//	rdusedw,
//	wrfull,
//	wrusedw);

routerDCFIFO top_in_FIFO (
	top_in,
	clk,
	top_read,
	top_in_clk,
	top_in_wr,//FILL THIS IN
	top_in_from_FIFO,
	top_in_FIFO_empty,
	,//rdusedw not used
	top_in_full,
	 //FILL THIS IN
);

routerDCFIFO BD_in_FIFO (
	BD_in,
	clk,
	BD_read,
	BD_in_clk,//FILL THIS IN
	BD_in_wr,//FILL THIS IN
	BD_in_from_FIFO,
	BD_in_FIFO_empty,
	,//rdusedw not used
	BD_in_full,
	 //FILL THIS IN
);

routerDCFIFO bot_in_FIFO (
	bot_in,
	clk,
	bot_read,
	bot_in_clk,
	bot_in_wr,//FILL THIS IN
	bot_in_from_FIFO,
	bot_in_FIFO_empty,
	,//rdusedw not used
	bot_in_full,
	 //FILL THIS IN
);

//3 output FIFOs (Same DCFIFO IP)
routerDCFIFO top_out_FIFO (
	top_out_to_FIFO,
	clk,//FILL THIS IN
	,//FILL THIS IN
	clk,
	top_wr,
	top_out,
	,//FILL THIS IN
	,//FILL THIS IN
	top_out_FIFO_full,
	//wrusedw not used
);

routerDCFIFO BD_out_FIFO (
	BD_out_to_FIFO,
	clk,//FILL THIS IN
	,//FILL THIS IN
	clk,
	BD_wr,
	BD_out,
	,//FILL THIS IN
	,//FILL THIS IN
	BD_out_FIFO_full,
	//wrusedw not used
);

routerDCFIFO bot_out_FIFO (
	bot_out_to_FIFO,
	clk,//FILL THIS IN
	,//FILL THIS IN
	clk,
	bot_wr,
	bot_out,
	,//FILL THIS IN
	,//FILL THIS IN
	bot_out_FIFO_full,
	//wrusedw not used
);

endmodule

