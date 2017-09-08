// PipeTest.v
//
// This is simple HDL that implements barebones PipeIn and PipeOut 
// functionality.  The logic generates and compares againt a pseudorandom 
// sequence of data as a way to verify transfer integrity and benchmark the pipe 
// transfer speeds.
//
// Copyright (c) 2005-2011  Opal Kelly Incorporated
// $Rev: 6 $ $Date: 2014-05-02 08:45:29 -0700 (Fri, 02 May 2014) $
//------------------------------------------------------------------------
`default_nettype none

module OKCoreTestHarness (
	input  wire [4:0]   okUH,
	output wire [2:0]   okHU,
	inout  wire [31:0]  okUHU,
	inout  wire         okAA,

  input wire user_reset,
	output wire [3:0]   led
	);

// OK stuff

// Target interface bus:
wire         okClk;
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
wire        pipe_in_write;
wire        pipe_in_ready;
wire [31:0] pipe_in_data;

// Pipe Out
wire        pipe_out_read;
wire        pipe_out_ready;
wire [31:0] pipe_out_data;

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

localparam NPCcode = 8;
localparam NPCdata = 24;
localparam NPCinout = NPCcode + NPCdata;
localparam Nfifo = 9; // 512-word FIFO
localparam Nblock = 7; // 128-word block size for BTPipe
localparam logic [NPCcode-1:0] NOPcode = 64; // upstream nop code
localparam PCtoBDcode = {NPCcode{1'b1}}; // downstream traffic with this code goes to the BDInput
localparam BDtoPCcode = {NPCcode{1'b1}}; // upstream traffic with this code came from BD

Channel #(NPCinout) PC_downstream();
Channel #(NPCinout) PC_upstream();

////////////////////////////////
// downstream: FIFO sinking BTPipeIn

logic [Nfifo-1:0] FIFO_in_count;
logic [NPCinout-1:0] FIFO_in_data_out;
logic FIFO_in_rd_ack;
logic FIFO_in_empty;

// input FIFO, compute ready signal using used
// if the MSBs aren't all one, there's 128 words free
assign pipe_in_ready = ~(&FIFO_in_count[Nfifo-1:Nblock]);

// if the Nblock bit is one, there's at least 128 entries
assign pipe_out_ready = FIFO_in_count[Nblock];

PipeInFIFO fifo_in(
  .data(pipe_in_data),
  .wrreq(pipe_in_write),
  .q(FIFO_in_data_out),
  .rdreq(FIFO_in_rd_ack),
  .empty(FIFO_in_empty),
  .usedw(FIFO_in_count),
  .clock(okClk));

// handshake output channel
assign PC_downstream.v = ~FIFO_in_empty;
assign PC_downstream.d = FIFO_in_data_out;
assign FIFO_in_rd_ack = PC_downstream.a;

///////////////////////////////
// upstream: either supply nop or data (if available)

always_comb
  if (pipe_out_read == 1) // pipe needs data this cycle!
    if (PC_upstream.v == 1) begin // use data from channel
      pipe_out_data = PC_upstream.d;
      PC_upstream.a = 1;
    end
    else begin
      pipe_out_data = {NOPcode, {NPCdata{1'b0}}};
      PC_upstream.a = 0;
    end
  else begin
    pipe_out_data = 'X;
    PC_upstream.a = 0;
  end

CoreTestHarness #(NPCcode, PCtoBDcode, BDtoPCcode) core_harness(PC_downstream, PC_upstream, okClk, user_reset);

endmodule
