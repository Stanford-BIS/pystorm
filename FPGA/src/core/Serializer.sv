`ifndef SERIALIZER_SV
`define SERIALIZER_SV

`include "../lib/Channel.svh"

module Serializer #(parameter Nin = 2, parameter Nout = 1) (
  Channel out,
  Channel in,
  input clk, reset);

// make a wide channel into multiple transmissions of a narrow channel
// send LSBs first
// if received {a, b, c}; send {c}, {b}, {a}

// D = serialization factor
// no internal registers, just make the input channel hold while we serialize
localparam D = Nin % Nout == 0 ? Nin / Nout : Nin / Nout + 1;

// one state per D
logic [$clog2(D)-1:0] state;
logic [$clog2(D)-1:0] next_state;
logic [$clog2(D)-1:0] state_p1;

assign state_p1 = state + 1;

always_ff @(posedge clk, posedge reset)
  if (reset == 1)
    state <= 0;
  else 
    state <= next_state;

always_comb
  case (state)
    default: 
      if (out.a == 1)
        next_state = state_p1;
      else
        next_state = state;
    D-1:
      if (out.a == 1)
        next_state = 0;
      else
        next_state = state;
  endcase

assign out.v = in.v;

logic [D-1:0][Nout-1:0] out_branch;
genvar i;
generate
  for (i = 0; i < D; i++) begin : out_branch_generate
    if (Nout * (i + 1) <= Nin)
      assign out_branch[i] = in.d[Nout*(i + 1)-1:Nout*i];
    else
      assign out_branch[i] = in.d[Nin-1:Nout*i];
  end
endgenerate
assign out.d = out_branch[state];

// handshake input
assign in.a = out.a & (state == D-1); // this means that we finished sending a word

endmodule

`endif
