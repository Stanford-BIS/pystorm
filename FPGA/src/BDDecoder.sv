`include "Channel.svh"
`include "ChannelUtil.svh"
`include "Interfaces.svh"
`include "Deserializer.sv"

// BD funnel routing table
/*
BDHornEP |leaf name         |route           |rt len |serialization |data width  |
=========|==================|================|=======|==============|============|===========================
0        |DUMP_AM           |100100000000000 |15     |2             |19          |AM diagnostic read output
1        |DUMP_MM           |100100000000001 |15     |1             |8           |MM diagnostic read output
2        |DUMP_PAT          |10010000000001  |14     |1             |20          |PAT diagnostic read output
3        |DUMP_POST_FIFO[0] |100100000001100 |15     |1             |19          |copy of tag class 0 traffic exiting FIFO
4        |DUMP_POST_FIFO[1] |100100000001101 |15     |1             |19          |copy of tag class 1 traffic exiting FIFO
5        |DUMP_PRE_FIFO     |10010000000101  |14     |1             |20          |copy of traffic entering FIFO
6        |DUMP_TAT[0]       |10000           |5      |1             |29          |TAT 0 diagnostic read output
7        |DUMP_TAT[1]       |10001           |5      |1             |29          |TAT 1 diagnostic read output
8        |NRNI              |101             |3      |1             |12          |copy of traffic exiting neuron array
9        |OVFLW[0]          |10010000000100X |14+    |1             |1           |class 0 FIFO overflow warning
10       |OVFLW[1]          |10010000000100X |14+    |1             |1           |class 1 FIFO overflow warning
11       |RO_ACC            |01              |2      |1             |28          |tag output from accumulator
12       |RO_TAT            |00              |2      |1             |32          |tag output from TAT

we haven't actually observed the OVFLW outputs, but the part of the route we're able to infer is enough
*/

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
// BDHornDecoder decodes BD route -> BDHornEP
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
module BDHornDecoder (
  DecodedBDWordChannel dec_out,
  Channel BD_in);


// BD output word format:
// [ route | X | payload]

// this module isn't really parametrized, it only works for this width
localparam NBDdata = 34;
localparam Nbiggest_single_payload = 32;
localparam Nfunnel = 13;
localparam Ncode = 4;

///////////////////////////////////////////
// reinterpretation of table data

enum {
  DUMP_AM,
  DUMP_MM,
  DUMP_PAT,
  DUMP_POST_FIFO0,
  DUMP_POST_FIFO1,
  DUMP_PRE_FIFO,
  DUMP_TAT0,
  DUMP_TAT1,
  NRNI,
  OVFLW0,
  OVFLW1,
  RO_ACC,
  RO_TAT,
  INVALID} leaf;

// leaf id's value is also the output code
const logic [0:Nfunnel-1][NBDdata-1:0] route_masks = '{
  {{15{1'b1}}, {(34-15){1'b0}}},
  {{15{1'b1}}, {(34-15){1'b0}}},
  {{14{1'b1}}, {(34-14){1'b0}}},
  {{15{1'b1}}, {(34-15){1'b0}}},
  {{15{1'b1}}, {(34-15){1'b0}}},
  {{14{1'b1}}, {(34-14){1'b0}}},
   {{5{1'b1}},  {(34-5){1'b0}}},
   {{5{1'b1}},  {(34-5){1'b0}}},
   {{3{1'b1}},  {(34-3){1'b0}}},
  {{14{1'b1}}, {(34-14){1'b0}}},
  {{14{1'b1}}, {(34-14){1'b0}}},
   {{2{1'b1}},  {(34-2){1'b0}}},
   {{2{1'b1}},  {(34-2){1'b0}}}};

const logic [0:Nfunnel-1][NBDdata-1:0] payload_masks = ~route_masks;

const logic [0:Nfunnel-1][NBDdata-1:0] routes = '{
  'b100100000000000 << (NBDdata - 15),
  'b100100000000001 << (NBDdata - 15),
  'b10010000000001  << (NBDdata - 14),
  'b100100000001100 << (NBDdata - 15),
  'b100100000001101 << (NBDdata - 15),
  'b10010000000101  << (NBDdata - 14),
  'b10000           << (NBDdata - 5 ),
  'b10001           << (NBDdata - 5 ),
  'b101             << (NBDdata - 3 ),
  'b10010000000100  << (NBDdata - 14),
  'b10010000000100  << (NBDdata - 14),
  'b01              << (NBDdata - 2 ),
  'b00              << (NBDdata - 2 )};

///////////////////////////////////////////
// logic

// do funnel decode in parallel
logic [Nfunnel-1:0][NBDdata-1:0] masked_routes;
logic [Nfunnel-1:0] test;
genvar i;
generate
for (i = 0; i < Nfunnel; i++) begin : masked_routes_generate
  assign masked_routes[i] = BD_in.d & route_masks[i];
  assign test[i] = (masked_routes[i] == routes[i]);
end
endgenerate

// one-hot -> binary (enum)
always_comb
  unique case (test)
    13'b0000000000001:
      leaf = DUMP_AM;
    13'b0000000000010:
      leaf = DUMP_MM;
    13'b0000000000100:
      leaf = DUMP_PAT;
    13'b0000000001000:
      leaf = DUMP_POST_FIFO0;
    13'b0000000010000:
      leaf = DUMP_POST_FIFO1;
    13'b0000000100000:
      leaf = DUMP_PRE_FIFO;
    13'b0000001000000:
      leaf = DUMP_TAT0;
    13'b0000010000000:
      leaf = DUMP_TAT1;
    13'b0000100000000:
      leaf = NRNI;
    13'b0011000000000: // will match both OVFLWs until we figure out the actual route
      leaf = OVFLW0;
    13'b0100000000000:
      leaf = RO_ACC;
    13'b1000000000000:
      leaf = RO_TAT;
    13'b0000000000000:
      leaf = INVALID;
  endcase

logic [NBDdata-1:0] masked_payload;
assign masked_payload = BD_in.d & payload_masks[leaf]; // will not discard any extra "high" bits!
assign dec_out.payload = masked_payload[Nbiggest_single_payload-1:0]; // will 0-extend

assign dec_out.leaf_code = leaf; // enum -> int

assign dec_out.v = BD_in.v;
assign BD_in.a = dec_out.a;

endmodule

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
// BDHornDeserializer deserializes the two-part AMMM word
// which doesn't really work anyway, LOL
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

module BDHornDeserializer (
  DecodedBDWordChannel words_out,
  DecodedBDWordChannel words_in,
  input clk, reset);

localparam Nbiggest_payload = 38;
localparam Ncode = 4;
localparam N = Nbiggest_payload + Ncode;

// split off DUMP_AM words
localparam logic [3:0] DUMP_AM_code = 0;

//////////////////////////////////
// pack input
Channel #(N) words_in_packed();
assign words_in_packed.d = {words_in.leaf_code, words_in.payload};
assign words_in_packed.v = words_in.v;
assign words_in.a = words_in_packed.a;

//////////////////////////////////
// split off DUMP_AM

Channel #(N) DUMP_AM_words();
Channel #(N) other_words();
ChannelSplit #(.N(N), .Mask({{Ncode{1'b1}}, {Nbiggest_payload{1'd0}}}), .Code0({DUMP_AM_code, {Nbiggest_payload{1'd0}}}))
  split(DUMP_AM_words, other_words, words_in_packed);
 

//////////////////////////////////
// deserialize DUMP_AM

// chop off AM code
Channel #(19) DUMP_AM_data();
assign DUMP_AM_data.v = DUMP_AM_words.v;
assign DUMP_AM_data.d = DUMP_AM_words.d[18:0];
assign DUMP_AM_words.a = DUMP_AM_data.a;

Channel #(38) DUMP_AM_data_deser();
Deserializer #(.Nin(19), .Nout(38)) des(DUMP_AM_data_deser, DUMP_AM_data, clk, reset);

// put the code back
Channel #(N) DUMP_AM_data_deser_coded();
assign DUMP_AM_data_deser_coded.v = DUMP_AM_data_deser.v;
assign DUMP_AM_data_deser_coded.d = {DUMP_AM_code, DUMP_AM_data_deser.d};
assign DUMP_AM_data_deser.a = DUMP_AM_data_deser_coded.a;

//////////////////////////////////
// re-merge streams

Channel #(N) words_out_packed();
ChannelMerge merge(words_out_packed, other_words, DUMP_AM_data_deser_coded, clk, reset);

//////////////////////////////////
// unpack output
assign {words_out.leaf_code, words_out.payload} = words_out_packed.d;
assign words_out.v = words_out_packed.v;
assign words_out_packed.a = words_out.a;

endmodule

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
// BDDecoder combines BDHornDecoder and BDHornDeserializer
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

module BDDecoder (
  DecodedBDWordChannel dec_out,
  Channel BD_in,
  input clk, reset);

DecodedBDWordChannel funnel_dec_out();
BDHornDecoder funnel_dec(funnel_dec_out, BD_in);
BDHornDeserializer funnel_des(dec_out, funnel_dec_out, clk, reset);

endmodule
