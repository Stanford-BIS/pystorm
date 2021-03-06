`ifndef DESERIALIZER_SV
`define DESERIALIZER_SV

`include "../lib/Channel.svh"

module Deserializer #(parameter Nin = 1, parameter Nout = 2) (
  Channel out, // Nout wide
  Channel in, // Nin wide
  input clk, reset);

// make a wide channel from multiple transmissions of a narrow channel.
// Send LSBs first
// if order into in is {a}, {b}, {c}; packed word looks like {c, b, a}

// D = number of registers/serialization factor
localparam D = Nout % Nin == 0 ? Nout / Nin : Nout / Nin + 1;

// one state per D
enum {LATCH, SEND} state, next_state;
logic [$clog2(D)-1:0] reg_id;
logic [$clog2(D)-1:0] next_reg_id;
logic [$clog2(D)-1:0] reg_id_p1;

assign reg_id_p1 = reg_id + 1;

always_ff @(posedge clk, posedge reset)
  if (reset == 1) begin
    state <= LATCH;
    reg_id <= 0;
  end
  else begin
    state <= next_state;
    reg_id <= next_reg_id;
  end

always_comb
  unique case (state)
    LATCH:
      if (in.v == 1 && reg_id == D-1) begin // latch the last chunk this state, send next state
        next_state <= SEND;
        next_reg_id <= 'X;
      end
      else if (in.v == 1) begin
        next_state <= LATCH;
        next_reg_id <= reg_id_p1;
      end
      else begin
        next_state <= LATCH;
        next_reg_id <= reg_id;
      end
    SEND:
      if (out.a == 1) begin
        next_state <= LATCH;
        next_reg_id <= 0;
      end
      else begin
        next_state <= SEND;
        next_reg_id <= 'X;
      end
  endcase

// registers
logic [D-1:0][Nin-1:0] chunk_reg; // send in LSBs -> MSBs
logic [Nin*D-1:0] chunk_reg_unpacked;
assign chunk_reg_unpacked = chunk_reg;
always_ff @(posedge clk, posedge reset)
  if (reset == 1)
    chunk_reg <= 0;
  else
    for (int i = 0; i < D; i++)
      if (state == LATCH && i == reg_id && in.v == 1)
        chunk_reg[i] <= in.d;
      else
        chunk_reg[i] <= chunk_reg[i];

// drive output
always_comb
  if (state == SEND) begin
    out.v = 1;
    out.d = chunk_reg_unpacked[Nout-1:0]; // discard any extra bits
  end
  else begin
    out.v = 0;
    out.d = 'X;
  end

// handshake input
assign in.a = (state == LATCH) & in.v;

endmodule

`endif
