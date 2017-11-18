`include "Channel.svh"

// for quartus, we add external IP to the project
`ifdef SIMULATION
  `include "../quartus/PipeInFIFO.v"
  `include "../quartus/PipeOutFIFO.v"
  `include "../ext/opalkelly/Simulation/glbl.v"
  `include "../ext/opalkelly/Simulation/okHost.v"
  `include "../ext/opalkelly/Simulation/okBTPipeOut.v"
  `include "../ext/opalkelly/Simulation/okBTPipeIn.v"
  `include "../ext/opalkelly/Simulation/okPipeOut.v"
  `include "../ext/opalkelly/Simulation/okPipeIn.v"
  `include "../ext/opalkelly/Simulation/okWireOR.v"
`endif

// OK stuff is a little crazy, apparently
`resetall

module OKIfc #(
  parameter NPCcode = 8,
  parameter logic [NPCcode-1:0] NOPcode = 64, // upstream nop code
  parameter logic [NPCcode-1:0] DSQueueCode = 128) // upstream nop code
(
	input  wire [4:0]   okUH,
	output wire [2:0]   okHU,
	inout  wire [31:0]  okUHU,
	inout  wire         okAA,
	output wire [3:0]   led,
  input wire [3:0]    led_in,
  output wire         okClk,
  input wire          user_reset,
  Channel             PC_downstream,
  Channel             PC_upstream);

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

assign led      = zem5305_led(led_in);

// Pipe In
logic        pipe_in_write;
logic        pipe_in_ready;
logic [31:0] pipe_in_data;
logic        pipe_in_blockstrobe;

// Pipe Out
logic        pipe_out_read;
logic        pipe_out_ready;
logic [31:0] pipe_out_data;
logic        pipe_out_blockstrobe;

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
  .ep_blockstrobe(pipe_in_blockstrobe),
  .ep_dataout(pipe_in_data),
  .ep_ready(pipe_in_ready));

// we're really just using BDPipeOut
// to get the blockstrobe, so we know
// when the beginning of a transmission happens
okBTPipeOut OK_pipe_out(
  .okHE(okHE),
  .okEH(okEHx[1*65+:65]),
  .ep_addr(8'ha0),
  .ep_blockstrobe(pipe_out_blockstrobe),
  .ep_datain(pipe_out_data),
  .ep_ready(pipe_out_ready),
  .ep_read(pipe_out_read));

////////////////////////////////
// channels to the rest of the design

localparam NPCdata = 24;
localparam NPCinout = NPCcode + NPCdata;
localparam Nfifo_in = 14; // 16K-word FIFO
localparam Nfifo_out = 14; // 16K-word FIFO
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
//assign pipe_out_ready = FIFO_in_count[Nblock];
assign pipe_out_ready = 1; // we're always ready, we might just transmit some empty blocks

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

// FSM controls transmission state:
// to save SW bandwidth, we transmit only NOPs after we drain the buffer once
// even if the HW puts additional words in after this point
// this way, the decoder can detect the first NOP in the block and process no further words
// in SEND_DS_QUEUE_STATE we report how full the downstream queue is
// this is used by comm to determine whether to execute a write
// if comm attempts to write when the downstream traffic is blocked (easily
// happens with wait events), it will take a long time for the call to
// complete and rob upstream bandwidth
enum {SEND_DS_QUEUE_STATE, SEND_DATA, SEND_NOPS} state, next_state;

always_ff @(posedge okClk, posedge user_reset)
  if (user_reset == 1)
    state <= SEND_NOPS;
  else
    state <= next_state;

always_comb
  unique case (state)
    SEND_NOPS:
      if (pipe_out_blockstrobe == 1)
        next_state = SEND_DS_QUEUE_STATE; // beginning of a block, send DS queue state
      else
        next_state = SEND_NOPS; // keep sending nops
    SEND_DS_QUEUE_STATE:
      if (FIFO_out_empty == 0)
        next_state = SEND_DATA; // have data, send it
      else
        next_state = SEND_NOPS; // no data to send
    SEND_DATA:
      if (pipe_out_blockstrobe == 1)
        next_state = SEND_DS_QUEUE_STATE; // beginning of a block, send DS queue state
      else if (FIFO_out_empty == 0)
        next_state = SEND_DATA; // keep sending
      else
        next_state = SEND_NOPS; // out of data, done sending until the next block
  endcase

// delay pipe out data 1 cycle
// per OK manual
logic [31:0] next_pipe_out_data;
always_ff @(posedge okClk, posedge user_reset)
  if (user_reset == 1)
    pipe_out_data <= 'X;
  else
    pipe_out_data <= next_pipe_out_data;

// FIFO -> pipe
always_comb
  if (pipe_out_read == 1) // pipe needs data next cycle
    if (state == SEND_DS_QUEUE_STATE) begin
      next_pipe_out_data = {DSQueueCode, {(NPCdata-Nfifo_in){1'b0}}, FIFO_in_count}; // downstream fifo queue occupancy
      FIFO_out_rd_ack = 0;
    end
    else if (state == SEND_NOPS) begin // make an entire block of NOPS
      next_pipe_out_data = {NOPcode, {NPCdata{1'b0}}};
      FIFO_out_rd_ack = 0;
    end
    else begin // state == SEND_DATA
      if (FIFO_out_empty == 0) begin // last cycle won't have data, so check
        next_pipe_out_data = FIFO_out_data_out;
        FIFO_out_rd_ack = 1;
      end
      else begin // otherwise, pad nops
        next_pipe_out_data = {NOPcode, {NPCdata{1'b0}}};
        FIFO_out_rd_ack = 0;
      end
    end
  else begin
    next_pipe_out_data = 'X;
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
