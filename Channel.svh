`ifndef CHANNEL_SVH
`define CHANNEL_SVH
// valid/data-acknowledge channel
// valid (v) triggers on posedge
// acknowledge (a) is generated with combinational logic, or negedge
// this scheme allows for full speed (one data transfer per cycle)
interface Channel #(parameter N = 1);
  logic [N-1:0] d;
  logic v;
  logic a;
endinterface

// valid/data-acknowledge channel, but no data
// used for a synchronization handshake
interface DatalessChannel;
  logic v;
  logic a;
endinterface

// valid/data channel (receiver must guarantee data is used in clk cycle v is high)
interface HalfChannel #(parameter N = 1);
  logic [N-1:0] d;
  logic v;
endinterface

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



// module that drives the and .d member of a DatalessChannel with 
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
// Based on DatalessChannelSrc
module RandomChannelSrc #(
  parameter N = 1, 
  parameter Max = 2**N-1, 
  parameter Min = 0,
  parameter ClkDelaysMin = 0,
  parameter ClkDelaysMax = 5) (Channel out, input clk, reset);

DatalessChannel base();
assign out.v = base.v;
assign base.a = out.a;

always @(base.v)
  if (base.v == 1)
    out.d <= $urandom_range(Max, Min); 
  else if (base.v == 0)
    out.d <= 'X;

DatalessChannelSrc #(.ClkDelaysMin(ClkDelaysMin), .ClkDelaysMax(ClkDelaysMax)) base_src(base, clk, reset);

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

always @(base.a)
  if (base.a == 1)
    $display("at %g: sunk %b", $time, in.d);

DatalessChannelSink #(.ClkDelaysMin(ClkDelaysMin), .ClkDelaysMax(ClkDelaysMax)) base_sink(base, clk, reset);

endmodule


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

`endif
