//BrainDrizzle Router Node Top
`include "InputController.sv"
`include "allocator.sv"
`include "interboard_input.sv"
`include "interboard_output.sv"
`ifdef SIMULATION
	`include "../../quartus/routerDCFIFO.v"
`endif


module BrainDrizzle(
input clk,
input reset, 
input top_in_clk,
input bot_in_clk,
input BD_in_clk,
input top_valid_in,
input bot_valid_in,
input BD_valid_in,
input top_ready_in,
input BD_ready_in,
input bot_ready_in,
input [10:0] top_in,
input [10:0] bot_in,
input [10:0] BD_in,
output [10:0] top_out,
output [10:0] bot_out,
output [10:0] BD_out,
output top_valid_out,
output bot_valid_out,
output BD_valid_out,
output top_ready_out,
output BD_ready_out,
output bot_ready_out,
output top_out_clk,
output BD_out_clk,
output bot_out_clk,
output sent_something
);

assign top_out_clk=clk;
assign BD_out_clk=clk;
assign bot_out_clk=clk;


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
wire [7:0] top_in_wrusedw;
wire [7:0] BD_in_wrusedw;
wire [7:0] bot_in_wrusedw;
//Signals which originate from output FIFOs
wire top_out_FIFO_full;
wire bot_out_FIFO_full;
wire BD_out_FIFO_full;
wire top_out_FIFO_empty;
wire BD_out_FIFO_empty;
wire bot_out_FIFO_empty;
wire [10:0] top_out_from_FIFO;
wire [10:0] BD_out_from_FIFO;
wire [10:0] bot_out_from_FIFO;
wire [7:0] top_out_rdusedw;
wire [7:0] BD_out_rdusedw;
wire [7:0] bot_out_rdusedw;

//Signals which originate from input handshaker
wire [10:0] top_to_input_FIFO;
wire [10:0] BD_to_input_FIFO;
wire [10:0] bot_to_input_FIFO;
wire top_in_wr;
wire BD_in_wr;
wire bot_in_wr;

//Signals which originate from output handshaker
wire top_out_read;
wire BD_out_read;
wire bot_out_read;




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
reset,
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
reset,
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
reset,
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
reset,
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
reset,
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
reset,
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
// aclr,
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
	reset,
	top_to_input_FIFO,
	clk,
	top_read,
	top_in_clk,
	top_in_wr,
	top_in_from_FIFO,
	top_in_FIFO_empty,
	,//rdusedw not used
	,//full not used
	top_in_wrusedw
);

routerDCFIFO BD_in_FIFO (
	reset,
	BD_to_input_FIFO,
	clk,
	BD_read,
	BD_in_clk,
	BD_in_wr,
	BD_in_from_FIFO,
	BD_in_FIFO_empty,
	,//rdusedw not used
	,//full not used
	BD_in_wrusedw
);

routerDCFIFO bot_in_FIFO (
	reset,
	bot_to_input_FIFO,
	clk,
	bot_read,
	bot_in_clk,
	bot_in_wr,
	bot_in_from_FIFO,
	bot_in_FIFO_empty,
	,//rdusedw not used
	,//full not used
	bot_in_wrusedw
);

//3 output FIFOs (Same DCFIFO IP)
routerDCFIFO top_out_FIFO (
	reset,
	top_out_to_FIFO,
	clk,
	top_out_read,
	clk,
	top_wr,
	top_out_from_FIFO,
	top_out_FIFO_empty,
	top_out_rdusedw,
	top_out_FIFO_full,
	//wrusedw not used
);

routerDCFIFO BD_out_FIFO (
	reset,
	BD_out_to_FIFO,
	clk,
	BD_out_read,
	clk,
	BD_wr,
	BD_out_from_FIFO,
	BD_out_FIFO_empty,
	BD_out_rdusedw,
	BD_out_FIFO_full,
	//wrusedw not used
);

routerDCFIFO bot_out_FIFO (
	reset,
	bot_out_to_FIFO,
	clk,
	bot_out_read,
	clk,
	bot_wr,
	bot_out_from_FIFO,
	bot_out_FIFO_empty,
	bot_out_rdusedw,
	bot_out_FIFO_full,
	//wrusedw not used
);

//sent_something
assign sent_something=~top_out_FIFO_empty;

//module interboard_input(
//	input transmit_clk, //clock from board we're receiving input from
//	input valid, //valid signal from the board we're recieving data from
//	input [10:0] recieve_data, //data recieved from the other board
//	input [7:0] wrusedw, //fifo words remaining
//	input reset,
//	output reg [10:0] data, //data sent to the fifo
//	output reg wrreq, //write request for fifo
//	output reg read, //read request for next board
//	);

interboard_input top_in_handshaker (
	top_in_clk,
	top_valid_in,
	top_in,
	top_in_wrusedw,
	reset,
	top_to_input_FIFO,
	top_in_wr,
	top_ready_out
);

interboard_input BD_in_handshaker (
	BD_in_clk,
	BD_valid_in,
	BD_in,
	BD_in_wrusedw,
	reset,
	BD_to_input_FIFO,
	BD_in_wr,
	BD_ready_out
);

interboard_input bot_in_handshaker (
	bot_in_clk,
	bot_valid_in,
	bot_in,
	bot_in_wrusedw,
	reset,
	bot_to_input_FIFO,
	bot_in_wr,
	bot_ready_out
);



//module interboard_output(
//	input [10:0] fifo_data, //data we're reading from the fifo (q port)
//	input read_input, //read signal from the board we're transmitting to
//	input empty, //if read is empty
//	input input_clk, //faster clock from this board
//	input reset,
//	output reg valid, //valid signal, sent to the board we're transmitting to
//	output reg [10:0] send_data, //data sent to next board
//	output reg rdreq //read request for dual clock fifo
//	);

interboard_output top_out_handshaker (
	top_out_from_FIFO,
	top_ready_in,
	top_out_FIFO_empty,
	clk,
	reset,
	top_valid_out,
	top_out,
	top_out_read
);

interboard_output BD_out_handshaker (
	BD_out_from_FIFO,
	BD_ready_in,
	BD_out_FIFO_empty,
	clk,
	reset,
	BD_valid_out,
	BD_out,
	BD_out_read
);

interboard_output bot_out_handshaker (
	bot_out_from_FIFO,
	bot_ready_in,
	bot_out_FIFO_empty,
	clk,
	reset,
	bot_valid_out,
	bot_out,
	bot_out_read
);
endmodule

