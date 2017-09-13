`include "Channel.svh"

`ifdef SIMULATION
`include "../ext/opalkelly/Simulation/okHost.v"
`include "../ext/opalkelly/Simulation/okBTPipeOut.v"
`include "../ext/opalkelly/Simulation/okBTPipeIn.v"
`include "../ext/opalkelly/Simulation/okPipeOut.v"
`include "../ext/opalkelly/Simulation/okPipeIn.v"
`else
`include "../ext/opalkelly/FrontPanelHDL/okHost.sv"
`include "../ext/opalkelly/FrontPanelHDL/okHost.v"
`include "../ext/opalkelly/FrontPanelHDL/okLibrary.v"
`include "../ext/opalkelly/FrontPanelHDL/okEndpoints.v"
`endif

`include "../quartus/PipeInFIFO.v"
`include "../quartus/PipeOutFIFO.v"

`default_nettype none

module OKIfc #(
  parameter NPCcode = 8,
  parameter logic [NPCcode-1:0] NOPcode = 64) // upstream nop code
(
	input  wire [4:0]   okUH,
	output wire [2:0]   okHU,
	inout  wire [31:0]  okUHU,
	inout  wire         okAA,
	output wire [3:0]   led,
  output wire okClk,
  input wire user_reset,
  Channel PC_downstream,
  Channel PC_upstream);

////////////////////////////////////////
// OK stuff

// Target interface bus:
wire [112:0] okHE;
wire [64:0]  okEH;

function [3:0] zem5305_led;
input [3:0] a;
integer i;
begin
	for(i=0; i<4; i=i+1) begin: u
		zem5305_led[i] = (a[i]==1'b1) ? (1'b0) : (1'bz);
	end
end
endfunction

assign led      = zem5305_led(4'b1);

// Pipe In
logic        pipe_in_write;
logic        pipe_in_ready;
logic [31:0] pipe_in_data;

// Pipe Out
logic        pipe_out_read;
logic        pipe_out_ready;
logic [31:0] pipe_out_data;

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

okBTPipeIn OK_pipe_in(
  .okHE(okHE),
  .okEH(okEHx[ 0*65 +: 65 ]),
  .ep_addr(8'h80),
  .ep_write(pipe_in_write),
  .ep_blockstrobe(),
  .ep_dataout(pipe_in_data),
  .ep_ready(pipe_in_ready));

okPipeOut OK_pipe_out(
  .okHE(okHE),
  .okEH(okEHx[1*65+:65]),
  .ep_addr(8'ha0),
  .ep_datain(pipe_out_data),
  .ep_read(pipe_out_read));

//okBTPipeOut epA0(
//  .okHE(okHE),
//  .okEH(okEHx[ 1*65 +: 65 ]),
//  .ep_addr(8'ha0),
//  .ep_read(pipe_out_read),
//   .ep_blockstrobe(),
//  .ep_datain(pipe_out_data),
//  .ep_ready(pipe_out_ready));

////////////////////////////////
// channels to the rest of the design

localparam NPCdata = 24;
localparam NPCinout = NPCcode + NPCdata;
localparam Nfifo_in = 9; // 512-word FIFO
localparam Nfifo_out = 10; // 1024-word FIFO
localparam Nblock = 7; // 128-word block size for BTPipe

////////////////////////////////
// downstream: FIFO sinking BTPipeIn

logic [Nfifo_in-1:0] FIFO_in_count;
logic [NPCinout-1:0] FIFO_in_data_out;
logic FIFO_in_rd_ack;
logic FIFO_in_empty;
logic FIFO_in_full; // unused

// input FIFO, compute ready signal using used
// if the MSBs aren't all one, there's 128 words free
assign pipe_in_ready = ~(&FIFO_in_count[Nfifo_in-1:Nblock]);

// if the Nblock bit is one, there's at least 128 entries
assign pipe_out_ready = FIFO_in_count[Nblock];

PipeInFIFO fifo_in(
  .data(pipe_in_data),
  .wrreq(pipe_in_write),
  .q(FIFO_in_data_out),
  .rdreq(FIFO_in_rd_ack),
  .empty(FIFO_in_empty),
  .full(FIFO_in_full),
  .usedw(FIFO_in_count),
  .clock(okClk));

// handshake output channel
assign PC_downstream.v = ~FIFO_in_empty;
assign PC_downstream.d = FIFO_in_data_out;
assign FIFO_in_rd_ack = PC_downstream.a;

///////////////////////////////
// upstream: either supply nop or data (if available)

logic [Nfifo_out-1:0] FIFO_out_count; // unused
logic [NPCinout-1:0] FIFO_out_data_in;
logic [NPCinout-1:0] FIFO_out_data_out;
logic FIFO_out_rd_ack;
logic FIFO_out_wr;
logic FIFO_out_empty;
logic FIFO_out_full; 

PipeOutFIFO fifo_out(
  .data(FIFO_out_data_in),
  .wrreq(FIFO_out_wr),
  .q(FIFO_out_data_out),
  .rdreq(FIFO_out_rd_ack),
  .empty(FIFO_out_empty),
  .full(FIFO_out_full),
  .usedw(FIFO_out_count),
  .clock(okClk));

// FIFO -> pipe
always_comb
  if (pipe_out_read == 1) // pipe needs data this cycle!
    if (FIFO_out_empty == 0) begin // use data from FPGA
      pipe_out_data = FIFO_out_data_out;
      FIFO_out_rd_ack = 1;
    end
    else begin
      pipe_out_data = {NOPcode, {NPCdata{1'b0}}};
      FIFO_out_rd_ack = 0;
    end
  else begin
    pipe_out_data = 'X;
    FIFO_out_rd_ack = 0;
  end

// channel -> FIFO
always_comb
  if (PC_upstream.v == 1 && FIFO_out_full == 0) begin
    FIFO_out_wr = 1;
    PC_upstream.a = 1;
  end
  else begin
    FIFO_out_wr = 0;
    PC_upstream.a = 0;
  end

assign FIFO_out_data_in = PC_upstream.d;

endmodule
