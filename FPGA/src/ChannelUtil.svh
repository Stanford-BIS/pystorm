`ifndef CHANNEL_UTIL_SVH
`define CHANNEL_UTIL_SVH

`include "Channel.svh"

///////////////////////////////////////////
// Synthesizable channel helpers

////////////////////////////////////////////
// Unpack ChannelArray into array of Channels
// easier to pass ChannelArray in module port lists than array of Channels,
// but array of Channels is easier to work with internally

module UnpackChannelArray #(parameter M) (ChannelArray A, Channel B[M-1:0]);

genvar i;
generate
for (i = 0; i < M; i++) begin : UnpackChannelArray_generate
  assign B[i].d = A.d[i];
  assign B[i].v = A.v[i];
  assign A.a[i] = B[i].a;
end
endgenerate

endmodule

////////////////////////////////////////////
// N-stage circular FIFO for Channels
// Can be used to pipeline components that communicate
// over Channels. Breaks up long delay paths.
module ChannelFIFO #(parameter D = 2, parameter N = 1) (
  Channel out, 
  Channel in,
  input clk, reset);

localparam logD = $clog2(D);

logic [D-1:0][N-1:0] data;

logic [logD-1:0] head, head_p1, tail, tail_p1; 
assign head_p1 = head + 1;
assign tail_p1 = tail + 1;

// head is where you should read from
// tail is where you should write to
// therefore, 
// with one entry: head + 1 == tail
// empty: tail == head
// full: tail + 1 == head

logic full, empty;
assign empty = (tail == head);
assign full = (tail_p1 == head);

// tail update
always_ff @(posedge clk, posedge reset)
  if (reset == 1)
    tail <= 0;
  else
    if (full == 0 && in.v == 1)
      tail <= tail_p1;
    else
      tail <= tail;

// head update
always_ff @(posedge clk, posedge reset)
  if (reset == 1)
    head <= 0;
  else
    if (empty == 0 && out.a == 1)
      head <= head_p1;
    else
      head <= head;

// data write
always_ff @(posedge clk, posedge reset)
  if (reset == 1)
    data <= '0;
  else
    for (int i = 0; i < D; i++) 
      if (i == tail && full == 0 && in.v == 1)
        data[i] <= in.d;
      else
        data[i] <= data[i];

// input handshake
assign in.a = in.v & ~full;

// data read/output handshake
always_comb
  if (empty == 0) begin
    out.v = 1;
    out.d = data[head];
  end
  else begin
    out.v = 0;
    out.d = 'X;
  end
    
endmodule


////////////////////////////////////////////
// block channel transmission with stall signal

module ChannelStaller (
  Channel out,
  Channel in,
  input stall);

// only thing we have to do is hide the .v signal until not stalling
assign out.v = in.v & ~stall;
assign out.d = in.d;
assign in.a = out.a;

endmodule

////////////////////////////////////////////
// Merge two channels
// tries to be fair when heavily contested

module ChannelMerge (
  Channel out,
  Channel in0, in1,
  input clk, reset);

logic sel, last_sel;
always_ff @(posedge clk, posedge reset)
  if (reset == 1)
    last_sel <= 0;
  else
    if (out.a == 1)
      last_sel <= sel;

// note: output can switch when going from
// single-input-valid to both-inputs-valid
always_comb
  if (in0.v == 1 && in1.v == 0)
    sel = 0;
  else if (in0.v == 0 && in1.v == 1)
    sel = 1;
  else if (in0.v == 1 && in1.v == 1)
    sel = ~last_sel;
  else
    sel = last_sel;

assign out.v = in0.v | in1.v;
assign out.d = (sel == 0) ? in0.d : in1.d;
assign in0.a = (sel == 0) & out.a;
assign in1.a = (sel == 1) & out.a;

endmodule

////////////////////////////////////////////
// fork a channel two ways
// Mask is the range of bits to evaluate
// Code0 is the pattern that sends in to out0,
// any other patterns go to out1

module ChannelSplit #(parameter N = -1, parameter logic [N-1:0] Mask = 0, parameter logic [N-1:0] Code0 = 0) (
  Channel out0, out1, in);

logic sel0;
assign sel0 = ((in.d & Mask) == Code0);

assign out0.v = sel0 & in.v;
assign out0.d = sel0 ? in.d : 'x;
assign out1.v = ~sel0 & in.v;
assign out1.d = ~sel0 ? in.d : 'x;
assign in.a = out0.a | out1.a;

endmodule

////////////////////////////////////////////
// turn a low-frequency pulse + data into a Channel

module PulseToChannel #(parameter N = 1) (
  Channel out, // N wide
  input [N-1:0] data,
  input pulse,
  input clk, reset);

// assumed to be a relatively infrequent pulse
// if receiver doesn't ack in time, a pulse can be missed

enum {READY, SENDING} state, next_state;

always_ff @(posedge clk, posedge reset)
  if (reset == 1)
    state <= READY;
  else
    state <= next_state;

always_comb
  unique case (state)
    READY:
      if (pulse == 1)
        next_state = SENDING;
      else
        next_state = READY;
    SENDING:
      if (out.a == 1)
        next_state = READY;
      else
        next_state = SENDING;
  endcase

always_comb
  unique case (state)
    READY:
      out.d = 'X;
    SENDING:
      out.d = data;
  endcase

assign out.v = (state == SENDING);

endmodule

`endif
