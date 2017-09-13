`include "../src/Channel.svh"

////////////////////////////////////////////
// Testbench code
// (unsynthesizable)

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
      next_delay_ct = $urandom_range(ClkDelaysMax, ClkDelaysMin);

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
      next_delay_ct = delay_ct - 1;
    end
  end
  else begin
    if (out.a == 1) begin
      if (delay_ct <= 0) begin
        next_out_v = 1;
        next_delay_ct = $urandom_range(ClkDelaysMax, ClkDelaysMin);

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
    else begin
      next_out_v = 1;
      next_out_d = out.d;
      next_delay_ct = delay_ct;
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
