`ifndef CHANNEL_SVH
`define CHANNEL_SVH
// valid/data-acknowledge channel
// valid (v) triggers on posedge
// acknowledge (a) is generated with combinational logic, or negedge
// this scheme allows for full speed (one data transfer per cycle)
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

interface PassiveChannel #(parameter N = -1);
  logic [N-1:0] d;
  logic r;
  logic v;
endinterface

// valid/data-acknowledge channel, but no data
// used for a synchronization handshake
interface DatalessChannel;
  logic v;
  logic a;
endinterface

interface PassiveDatalessChannel;
  logic r;
  logic v;
endinterface

///////////////////////////////////////////
// Synthesizable channel helpers

////////////////////////////////////////////
// Unpack ChannelArray into array of Channels
// easier to pass ChannelArray in module port lists than array of Channels,
// but array of Channels is easier to work with internally

module UnpackChannelArray #(parameter M) (ChannelArray A, Channel B[M-1:0]);

generate
for (genvar i = 0; i < M; i++) begin
  assign B[i].d = A.d[i];
  assign B[i].v = A.v[i];
  assign A.a[i] = B[i].a;
end
endgenerate

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
assign out0.d = sel0 ? in.d : 'X;
assign out1.v = ~sel0 & in.v;
assign out1.d = ~sel0 ? in.d : 'X;
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

always_ff @(posedge clk, reset)
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

// module that drives the .v and .d member of a PassiveDatalessChannel with 
// random timings. Can parametrize to control range of delay times
module PassiveDatalessChannelSrc #(
  parameter ClkDelaysMin = 0,
  parameter ClkDelaysMax = 5) (PassiveDatalessChannel out, input clk, reset);

int next_delay;

always_ff @(posedge clk, posedge reset)
  if (reset == 1) begin
    out.v <= 0;
    next_delay <= 0;
  end
  else begin
    if (out.r == 0)
      out.v <= 0;
    else begin
      if (next_delay == 0) begin
        out.v <= 1;
        next_delay <= $urandom_range(ClkDelaysMax, ClkDelaysMin);
      end
      else begin
        next_delay <= next_delay - 1;
        out.v <= 0;
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
  parameter logic [N-1:0] Mask = (2**N-1)-1,
  parameter ClkDelaysMin = 0,
  parameter ClkDelaysMax = 5) (Channel out, input clk, reset);

parameter Chunks32 = N % 32 == 0 ? N / 32 : N / 32 + 1;

// delay_ct only counts down when we're 
int delay_ct;

enum {READY, HOLD} state, next_state;

always_ff @(posedge clk, posedge reset)
  if (reset == 1)
    state <= READY;
  else
    state <= next_state;

always_ff @(posedge clk, posedge reset)
  if (reset == 1)
    delay_ct <= $urandom_range(ClkDelaysMax, ClkDelaysMin);
  else
    if (state == READY && delay_ct <= 0)
      delay_ct <= $urandom_range(ClkDelaysMax, ClkDelaysMin);
    else
      delay_ct <= delay_ct - 1;

always_comb
  unique case (state)
    READY:
      if (delay_ct <= 0)
        if (out.a == 0)
          next_state = HOLD;
        else 
          next_state = READY;
      else
        next_state = READY;
    HOLD:
      if (out.a == 1)
        next_state = READY;
      else
        next_state = HOLD;
  endcase

always_comb
  unique case (state)
    READY:
      if (delay_ct <= 0) begin
        out.v = 1;

        // this would work for N < 32
        //out.d = $urandom_range(Max, Min);
        
       

        // for some reason this didn't work
        //for (int i = 0; i < Chunks32; i++) begin
        //  out.d[(32*(i+1))-1:32*i] = 
        //end
        
        // total hack
        assert (N < 128);
        out.d = {$urandom_range(2**32-1, 0), $urandom_range(2**32-1, 0), $urandom_range(2**32-1, 0), $urandom_range(2**32-1, 0)};

        // not uniform random if min/max is used
        if (out.d > Max)
          out.d = Max;
        if (out.d < Min)
          out.d = Min;

        out.d = out.d & Mask;
      end
      else begin
        out.v = 0;
        out.d = 'X;
      end
    HOLD:
      out.v = 1;

  endcase

endmodule

module RandomPassiveChannelSrc #(
  parameter N = 1, 
  parameter Max = 2**N-1, 
  parameter Min = 0,
  parameter ClkDelaysMin = 0,
  parameter ClkDelaysMax = 5) (PassiveChannel out, input clk, reset);

PassiveDatalessChannel base();
assign out.v = base.v;
assign base.r = out.r;

always @(base.v, posedge clk)
  if (base.v == 1)
    out.d <= $urandom_range(Max, Min); 
  else if (base.v == 0)
    out.d <= 'X;

PassiveDatalessChannelSrc #(.ClkDelaysMin(ClkDelaysMin), .ClkDelaysMax(ClkDelaysMax)) base_src(base, clk, reset);

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

// module that drives the .r members of a PassiveDatalessChannel
// uses random timings
module PassiveDatalessChannelSink #(
  parameter ClkDelaysMin = 0,
  parameter ClkDelaysMax = 5) (PassiveDatalessChannel in, input clk, reset);

int next_delay;

// r is meant to be assigned combinationally, can be delayed from posedge
// we emulate this by assigning on the negedge
always_ff @(negedge clk, posedge reset)
  if (reset == 1) begin
    next_delay <= 0;
    in.r <= 1;
  end
  else begin
    if (next_delay <= 0) begin
      in.r <= 1;
      if (in.v == 1) begin
        $display("at %g: sunk dataless channel", $time);
        next_delay <= $urandom_range(ClkDelaysMax, ClkDelaysMin);
      end
    end
    else begin
      next_delay <= next_delay - 1;
      in.r <= 0;
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
    $display("at %g: sunk %b", $time, in.d);


DatalessChannelSink #(.ClkDelaysMin(ClkDelaysMin), .ClkDelaysMax(ClkDelaysMax)) base_sink(base, clk, reset);

endmodule


module PassiveChannelSink #(
  parameter ClkDelaysMin = 0,
  parameter ClkDelaysMax = 5) (PassiveChannel in, input clk, reset);

PassiveDatalessChannel base();
assign base.v = in.v;
assign in.r = base.r;

always_ff @(posedge clk)
  if (base.v == 1)
    $display("at %g: sunk %b", $time, in.d);

PassiveDatalessChannelSink #(.ClkDelaysMin(ClkDelaysMin), .ClkDelaysMax(ClkDelaysMax)) base_sink(base, clk, reset);

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

///////////////////////////////////////
// TESTBENCH
module PassiveRandomChannel_tb;

parameter N = 4;
PassiveChannel #(.N(N)) chan();
logic clk;
logic reset = 0;

parameter D = 10;

always #(D) clk = ~clk;

initial begin
  clk = 0;
end

RandomPassiveChannelSrc #(.ClkDelaysMin(0), .ClkDelaysMax(4), .N(N)) src_dut(.out(chan), .*);
PassiveChannelSink sink_dut(.in(chan), .*);

endmodule

`endif
