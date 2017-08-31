`include "Channel.svh"
`include "Interfaces.svh"

module BDSerializer #(parameter Ncode = 8, parameter Ndata_out = 24) (
  SerializedPCWordChannel ser_out,
  DecodedBDWordChannel dec_in,
  input clk, reset);

// BD funnel routing table (from the wiki)
/*
|leaf name         |depth |route   |route(int) |data width |serialization |chunk width |
|==================|======|========|===========|===========|==============|============|===========================
|DUMP_AM           |6     |101000  |40         |38         |2             |19          |AM diagnostic read output
|DUMP_MM           |6     |101001  |41         |8          |1             |8           |MM diagnostic read output
|DUMP_PAT          |5     |10101   |21         |20         |1             |20          |PAT diagnostic read output
|DUMP_POST_FIFO[0] |6     |101110  |46         |19         |1             |19          |copy of tag class 0 traffic exiting FIFO
|DUMP_POST_FIFO[1] |6     |101111  |47         |19         |1             |19          |copy of tag class 1 traffic exiting FIFO
|DUMP_PRE_FIFO     |6     |101101  |45         |20         |1             |20          |copy of traffic entering FIFO
|DUMP_TAT[0]       |4     |1000    |8          |29         |1             |29          |TAT 0 diagnostic read output
|DUMP_TAT[1]       |4     |1001    |9          |29         |1             |29          |TAT 1 diagnostic read output
|NRNI              |2     |11      |3          |12         |1             |12          |copy of traffic exiting neuron array
|OVFLW[0]          |7     |1011000 |88         |1          |1             |1           |class 0 FIFO overflow warning
|OVFLW[1]          |7     |1011001 |89         |1          |1             |1           |class 1 FIFO overflow warning
|RO_ACC            |2     |01      |1          |28         |1             |28          |tag output from accumulator
|RO_TAT            |2     |00      |0          |32         |1             |32          |tag output from TAT
*/

parameter NBDpayload = 32; // width of DecodedBDWordChannel.payload (longest "data width")
parameter Nfunnel = 13; // number of funnel leaves

parameter int width_used[Nfunnel] = {
  19,
  8 ,
  20,
  19,
  19,
  20,
  29,
  29,
  12,
  1 ,
  1 ,
  28,
  32};

// have to assign parameters in one shot, at init, so need a fn
typedef int SerType[Nfunnel];
function SerType SerFromWidth(input int width_used[Nfunnel]);
  for (int i = 0; i < Nfunnel; i++)
    SerFromWidth[i] = width_used[i] % Ndata_out == 0 ? width_used[i] / Ndata_out : width_used[i] / Ndata_out + 1;
endfunction

parameter int serialization[Nfunnel] = SerFromWidth(width_used);

/////////////////////////////////////////

// you might need to add more states if Nout is really small compared to NBDdata
enum {S0, S1} state, next_state;

always_ff @(posedge clk, posedge reset)
  if (reset == 1)
    state <= S0;
  else
    state <= next_state;

always_comb
  unique case (state)
    S0:
      if (ser_out.a == 1)
        if (serialization[dec_in.leaf_code] > 1)
          next_state = S1;
        else 
          next_state = S0;
      else
        next_state = S0;

    S1: begin
      assert (serialization[dec_in.leaf_code] <= 2)
      if (ser_out.a == 1)
        next_state = S0;
      else
        next_state = S1;
    end
  endcase


// hold the input until we're done sending all parts
assign ser_out.v = dec_in.v;

parameter HiIdx0 = 1*Ndata_out < NBDpayload ? 1*Ndata_out : NBDpayload;
parameter HiIdx1 = 2*Ndata_out < NBDpayload ? 2*Ndata_out : NBDpayload;

assign ser_out.code = dec_in.leaf_code; // 0-extended
always_comb
  unique case (state)
    S0:
      ser_out.payload = dec_in.payload[HiIdx0-1 : 0*Ndata_out];
    S1:
      ser_out.payload = dec_in.payload[HiIdx1-1 : 1*Ndata_out];
  endcase

// handshake input
assign dec_in.a = ser_out.a & (next_state == S0); // this means that we finished sending a word

endmodule

///////////////////////////////
// TESTBENCH
module BDSerializer_tb;

DecodedBDWordChannel dec_in();
SerializedPCWordChannel ser_out();

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

Channel #(36) dec_in_packed();
assign dec_in.v = dec_in_packed.v;
assign {dec_in.leaf_code, dec_in.payload} = dec_in_packed.d;
assign dec_in_packed.a = dec_in.a;
RandomChannelSrc #(.N(36)) dec_in_src(dec_in_packed, clk, reset);

Channel #(16) ser_out_packed();
assign ser_out_packed.v = ser_out.v;
assign ser_out_packed.d = {ser_out.code, ser_out.payload};
assign ser_out.a = ser_out_packed.a;
ChannelSink ser_out_sink(ser_out_packed, clk, reset);

BDSerializer dut(.*);

endmodule
