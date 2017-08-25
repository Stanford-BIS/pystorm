`include "Channel.svh"

// dead-stupid round-robin arbiter with a counter
module RRArbiter #(parameter N = 2) (
  Channel out, 
  DatalessChannel in[N-1:0],
  input clk,
  input reset);


// can't index interfaces/modules with non-const
// map to logic array here
logic [N-1:0] in_vs;
generate
for (genvar i = 0; i < N; i++)
  assign in_vs[i] = in[i].v;
endgenerate

logic [$clog2(N)-1:0] count, count_p1, next_count;
enum {SENDING, NOT_SENDING} send_state, next_send_state;
// count/is_sending makes up the state vector

assign count_p1 = count + 1;

// update state
always_ff @(posedge clk, posedge reset)
  if (reset == 1) begin
    count <= 0;
    send_state <= NOT_SENDING;
  end
  else begin
    count <= next_count;
    send_state <= next_send_state;
  end

// compute next state
always_comb
  unique case (send_state)
  NOT_SENDING:
    // NOT_SENDING -> SENDING if in_vs[count + 1] == 1
    if (in_vs[count_p1] == 1) begin
      next_send_state = SENDING;
      next_count = count_p1;
    end
    // NOT_SENDING -> SENDING if in_vs[count + 1] == 0
    else begin
      next_send_state = NOT_SENDING;
      next_count = count_p1;
    end
  SENDING:
    if (out.a == 1) // done sending this message
      // SENDING -> SENDING if output ack and in_vs[count + 1] == 1 
      if (in_vs[count_p1] == 1) begin
        next_send_state = SENDING;
        next_count = count_p1;
      end
      // SENDING -> NOT_SENDING if output ack and in_vs[count + 1] == 0
      else begin
        next_send_state = NOT_SENDING;
        next_count = count_p1;
      end
    else begin
      // SENDING -> SENDING if no output ack
      // this is the only time you don't increment the count
      next_send_state = SENDING;
      next_count = count;
    end
  endcase

// assign output
always_comb
  unique case (send_state)
  NOT_SENDING: begin
    out.v = 0;
    out.d = 'X;
  end
  SENDING: begin
    out.v = 1;
    out.d = count;
  end
  endcase

// handshake input
generate
for (genvar i = 0; i < N; i++)
  always_comb
    if (i == count && send_state == SENDING && out.a == 1)
      in[i].a = 1;
    else
      in[i].a = 0;
endgenerate

//enum {COUNTING, SENDING} state, next_state;
//
//always_ff @(posedge clk, posedge reset)
//  if (reset == 1)
//    state <= COUNTING;
//  else
//    state <= next_state;
//
//logic send_condition;
//assign send_condition = curr_in_v;
//
//always_comb
//  unique case (state)
//  COUNTING:
//    if (send_condition == 1)
//      next_state <= SENDING;
//    else
//      next_state <= COUNTING;
//  SENDING:
//    if (out.a == 1)
//      if (send_condition == 1)
//        next_state <= SENDING;
//      else
//        next_state <= COUNTING;
//    else
//      next_state <= SENDING;
//  endcase
//
//always_comb
//  unique case (state)
//  COUNTING: begin
//    out.v = 0;
//    out.d = 'X;
//  end
//  SENDING: begin
//    out.v = 1;
//    out.d = count;
//  end
//  endcase
//
//// increment count, pause when trying to send
//always_ff @(posedge clk, posedge reset)
//  if (reset == 1)
//    count <= 0;
//  else
//    if (next_state == COUNTING)
//      count <= count + 1;
//
//// ack inputs combinationally (based on output ack!)
//// XXX this is a little sketchy
//// can get spurious ack pulses in the first half of the clk cycle
//generate
//for (genvar i = 0; i < N; i++)
//  always_comb
//    if (i == count && state == SENDING && out.a == 1)
//      in[i].a = 1;
//    else
//      in[i].a = 0;
//endgenerate

endmodule


// TESTBENCH
module RRArbiter_tb;

parameter Ninp = 4;
parameter Ndata = 4;

Channel #(Ndata) out(); 
DatalessChannel in[Ninp-1:0]();
logic clk;
logic reset;

parameter Tclk = 10;

always #(Tclk/2) clk = ~clk;

initial begin
  clk <= 0;
  reset <= 1;

  #(Tclk * 10) reset <= 0;
end

DatalessChannelSrc #(.ClkDelaysMin(Ninp*2), .ClkDelaysMax(Ninp*6)) sources[Ninp-1:0](in, clk, reset);
ChannelSink #(.ClkDelaysMin(0), .ClkDelaysMax(Ninp)) sink(out, clk, reset);

RRArbiter #(Ninp) dut(.out(out), .in(in), .clk(clk), .reset(reset));

endmodule

