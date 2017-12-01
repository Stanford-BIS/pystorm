`include "../lib/Channel.svh"
`include "../lib/Interfaces.svh"

module BDSerializer #(parameter Ncode = 8, parameter Ndata_out = 24) (
  SerializedPCWordChannel ser_out,
  DecodedBDWordChannel dec_in,
  input clk, reset);

// some BD words won't fit in a 24-bit FPGA payload
// BDHornEP 0  : DUMP_AM   : 38 bits (after BDFunnelDeserializer)
// BDHornEP 6  : DUMP_TAT0 : 29 bits
// BDHornEP 7  : DUMP_TAT1 : 29 bits
// BDHornEP 11 : RO_ACC    : 28 bits
// BDHornEP 12 : RO_TAT     : 32 bits

localparam NBDpayload = 38; // width of DecodedBDWordChannel.payload (longest "data width")
localparam Nfunnel = 14; // number of funnel leaves + INVALID

// actually serialization - 1
localparam logic serialization[0:Nfunnel-1] = '{
  1,
  0,
  0,
  0,
  0,
  0,
  1,
  1,
  0,
  0,
  0,
  1,
  1,
  1};


//// have to assign parameters in one shot, at init, so need a fn
//typedef int SerType[Nfunnel];
//function SerType SerFromWidth(input int width_used[Nfunnel]);
//  for (int i = 0; i < Nfunnel; i++)
//    SerFromWidth[i] = width_used[i] % Ndata_out == 0 ? width_used[i] / Ndata_out : width_used[i] / Ndata_out + 1;
//endfunction

logic serialization_sel;
assign serialization_sel = serialization[dec_in.leaf_code];

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
        if (serialization_sel == 1)
          next_state = S1;
        else 
          next_state = S0;
      else
        next_state = S0;

    S1: begin
      if (ser_out.a == 1)
        next_state = S0;
      else
        next_state = S1;
    end
  endcase


// hold the input until we're done sending all parts
assign ser_out.v = dec_in.v;

localparam HiIdx0 = 1*Ndata_out < NBDpayload ? 1*Ndata_out : NBDpayload;
localparam HiIdx1 = 2*Ndata_out < NBDpayload ? 2*Ndata_out : NBDpayload;

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
