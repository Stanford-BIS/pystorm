`ifndef CHANNEL_SVH
`define CHANNEL_SVH

// valid/data-acknowledge channel
//
// If valid (.v) goes high sometime in clock cycle i,
// that means that the data has a new, valid value on the next clock edge
// (the beginning of cycle i+1). 
// For full throughput, the receiver will assert acknowledge (.a) sometime
// after .v, still in clock cycle i. The receiver samples the
// data on the next clock edge (the beginning of cycle i+1).
// This scheme allows for full speed (one data transfer per cycle).
//
// Practically speaking, there's nothing preventing .v/.d from being
// register outputs, but they're often combinational functions of FSM states, 
// so I've marked them as transitioning a little after the clock edge in the
// following diagrams. There's nothing that says that .a can't be a register
// output, but for full throughput, it needs to be a purely combinational
// function of .v
//
// Timing example:
//
// .v |  ______|________|________|________|_       |
//    |_|      |        |        |        | |______|
//    |        |        |        |        |        |
// .d |_  _____|  ______|________|  ______|  ______|
//    |X\/  1  |\/   2  |   2    |\/   3  |\/   X  |
//    |_/\_____|/\______|________|/\______|/\______|
//    |        |        |        |        |        |
// .a |   _____|__      |   _____|________|__      |
//    |__|     |  |_____|__|     |        |  |_____|
//    |        |        |        |        |        |
//
//  1.receiver         3.then reads twice
//    reads once         (full throughput)
//              
//            2.then can't                 4.sender has
//              read for some                no more data
//              reason, doesn't ack
//
// Notes:
//
// .v |_   ____|________|
//    | |_|    |        |
//    |        |        |
// .d |________|  ______|
//    |   1    |\/   2  |
//    |________|/\______|
//    |        |        |
// .a |   _   _|________|
//    |__| |_| |        |
//    |        |        |
//
//    Technically the above is legal. We're not doing async handshakes with phases.
//    .a and .d just have to be high by the next clock edge
//    It's up to the synthesis tool to make sure everything works out.
//
// .v |        |  ______|
//    |________|_|      |
//    |        |        |
// .d |________|  ______|
//    |   X    |\/   1  |
//    |________|/\______|
//    |        |        |
// .a |   _____|__      |
//    |__|     |  |_____|
//    |        |        |
//
//    This, however, breaks the protocol.
//    .a should never be high on a clock edge that .v isn't high on
//
// .v |  ______|________|_       |
//    |_|      |        | |______|
//    |        |        |        |
// .d |_  _____|  ______|  ______|
//    |X\/  1  |\/   2  |\/   X  |
//    |_/\_____|/\______|/\______|
//    |        |        |        |
// .a |   _____|__      |        |
//    |__|     |  |_____|________|
//    |        |        |        |
//
//    It's legal for .v to go away without being acknowledged
//    (But I don't think I ever do this).
//    This only works if .a if being generated combinationally.
//
// .v |  ______|________|________|________|
//    |_|      |        |        |        |
//    |        |        |        |        |
// .d |_  _____|________|  ______|________|
//    |X\/  1  |    1   |\/   2  |    2   |
//    |_/\_____|________|/\______|________|
//    |        |        |        |        |
// .a |        |________|        |________|
//    |________|        |________|        |
//    |        |        |        |        |
//
//    If you know that .v isn't going to drop without being acked
//    (like it does in the previous example), there's nothing keeping
//    you from generating .a as a register output.
//    You're just never going to get more than half throughput.
//
interface Channel #(parameter N = -1);
  logic [N-1:0] d;
  logic v;
  logic a;
endinterface

interface ChannelArray #(parameter N = -1, parameter M = -1);
  logic [M-1:0][N-1:0] d;
  logic [M-1:0] v;
  logic [M-1:0] a;
endinterface

// valid/data-acknowledge channel, but no data
// used for a synchronization handshake
interface DatalessChannel;
  logic v;
  logic a;
endinterface

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
  case (state)
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

assign out.v = (state == SENDING);

endmodule

////////////////////////////////////////////
// Testbench code
// (unsynthesizable)

// can use for channel with data by making combinational function of valid
module ChannelSender (output logic valid, input ack, condition, clk, reset);

enum {WAITING, SENDING} state;

always_ff @(posedge clk, posedge reset)
  if (reset == 1)
    state <= WAITING;
  else
    unique case (state)
    WAITING:
      if (condition == 1)
        state <= SENDING;
      else
        state <= WAITING;
    SENDING:
      if (ack == 1)
        if (condition == 1)
          state <= SENDING;
        else
          state <= WAITING;
      else
        state <= SENDING;
    endcase

always_comb
  unique case (state)
  WAITING:
    valid = 0;
  SENDING:
    valid = 1;
  endcase

endmodule



// module that drives the .v and .d member of a DatalessChannel with
// random timings. Can parametrize to control range of delay times
module DatalessChannelSrc #(
  parameter ClkDelaysMin = 0,
  parameter ClkDelaysMax = 5) (DatalessChannel out, input clk, reset);

int next_delay;

initial begin
  out.v <= 0;
  wait (reset == 0);

  next_delay = $urandom_range(ClkDelaysMax, ClkDelaysMin);
  repeat (next_delay)
    @ (posedge clk);

  forever begin
    out.v <= 1;

    next_delay <= $urandom_range(ClkDelaysMax, ClkDelaysMin);

    @ (posedge clk);
    while (out.a == 0) begin
      next_delay <= next_delay - 1;
      @ (posedge clk);
    end

    if (next_delay > 0) begin
      out.v <= 0;
      repeat (next_delay - 1)
        @ (posedge clk);
    end
  end
end

endmodule


// module that drives the .v and .d members of a channel with
// random data, using random timings. Can parametrize to
// control range of random values and delay times.
module RandomChannelSrc #(
  parameter N = 1,
  parameter logic [N-1:0] Max = 2**N-1,
  parameter logic [N-1:0] Min = 0,
  parameter logic [N-1:0] Mask = 2**N-1,
  parameter ClkDelaysMin = 0,
  parameter ClkDelaysMax = 5) (Channel out, input clk, reset);

localparam Chunks32 = N % 32 == 0 ? N / 32 : N / 32 + 1;

int delay_ct, next_delay_ct;

logic [N-1:0] next_out_d;
logic next_out_v;

always_ff @(posedge clk, posedge reset)
  if (reset == 1) begin;
    out.v <= 0;
    out.d <= 'x;
    delay_ct <= $urandom_range(ClkDelaysMax, ClkDelaysMin);
  end
  else begin
    out.v <= next_out_v;
    out.d <= next_out_d;
    delay_ct <= next_delay_ct;
  end

always_comb
  if (out.v == 0) begin
    if (delay_ct <= 0) begin
      next_out_v = 1;

      assert (N < 128);
      next_out_d = {$urandom_range(2**32-1, 0), $urandom_range(2**32-1, 0), $urandom_range(2**32-1, 0), $urandom_range(2**32-1, 0)};

      // not uniform random if min/max is used
      if (out.d > Max)
        next_out_d = Max;
      if (out.d < Min)
        next_out_d = Min;

      next_out_d = next_out_d & Mask;
    end
    else begin
      next_delay_ct = delay_ct - 1;
    end
  end
  else begin
    if (out.a == 1) begin
      next_delay_ct = $urandom_range(ClkDelaysMax, ClkDelaysMin);
      if (delay_ct <= 0) begin
        next_out_v = 1;

        assert (N < 128);
        next_out_d = {$urandom_range(2**32-1, 0), $urandom_range(2**32-1, 0), $urandom_range(2**32-1, 0), $urandom_range(2**32-1, 0)};

        // not uniform random if min/max is used
        if (out.d > Max)
          next_out_d = Max;
        if (out.d < Min)
          next_out_d = Min;

        next_out_d = next_out_d & Mask;
      end
      else begin
        next_out_v = 0;
        next_out_d = 'X;
      end;
    end
  end

always @ (out.d)
  $display("[T=%g]: data=%h (RandomChannelSrc)", $time, out.d);

endmodule


// module that drives the .a members of a DatalessChannel
// uses random timings
module DatalessChannelSink #(
  parameter ClkDelaysMin = 0,
  parameter ClkDelaysMax = 5) (DatalessChannel in, input clk, reset);

int next_delay;

initial begin
  in.a <= 0;
  wait (reset == 0);

  forever begin

    next_delay <= $urandom_range(ClkDelaysMax, ClkDelaysMin);

    // a is meant to be assigned combinationally
    // we emulate this by assigning on the negedge
    @ (negedge clk);
    while (in.v == 0 || next_delay > 0) begin
      in.a <= 0;
      next_delay <= next_delay - 1;
      @ (negedge clk);
    end

    in.a <= 1;
    $display("at %g: sunk dataless channel", $time);
  end
end
endmodule


// module that drives the .a members of a channel.
// uses random timings, reports sunk data
// Based on DatalessChannelSink
module ChannelSink #(
  parameter ClkDelaysMin = 0,
  parameter ClkDelaysMax = 5) (Channel in, input clk, reset);

DatalessChannel base();
assign base.v = in.v;
assign in.a = base.a;

always_ff @(posedge clk)
  if (base.a == 1)
    //$display("at %g: sunk %b", $time, in.d);
    $display("[T=%g]: data=%h (ChannelSink)", $time, in.d);


DatalessChannelSink #(.ClkDelaysMin(ClkDelaysMin), .ClkDelaysMax(ClkDelaysMax)) base_sink(base, clk, reset);

endmodule


///////////////////////////////////////
// TESTBENCH
// hooks a RandomChannelSrc to a ChannelSink
module RandomChannel_tb;

parameter N = 4;
Channel #(.N(N)) chan();
logic clk;
logic reset = 0;

parameter D = 10;

always #(D) clk = ~clk;

initial begin
  clk = 0;
end

RandomChannelSrc #(.N(N)) src_dut(.out(chan), .*);
ChannelSink sink_dut(.in(chan), .*);

endmodule

// FIFO test
module ChannelFIFO_tb;

parameter D = 4; 
parameter N = 4;

Channel #(N) out();
Channel #(N) in();

// clock
logic clk;
parameter Tclk = 10;
always #(Tclk/2) clk = ~clk;
initial clk = 0;

// reset
logic reset;
initial begin
  reset <= 0;
  @(posedge clk) reset <= 1;
  @(posedge clk) reset <= 0;
end

RandomChannelSrc #(.N(N)) src(in, clk, reset);
ChannelSink sink(out, clk, reset);

ChannelFIFO #(.D(D), .N(N)) dut(.*);

endmodule

`endif
