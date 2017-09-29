`include "Channel.svh"
`include "ChannelUtil.svh"
`include "Interfaces.svh"
`include "Deserializer.sv"

// BD funnel routing table (from the wiki)
/*
BDHornEP |leaf name         |depth |route   |route(int) |data width |serialization |chunk width |
=========|==================|======|========|===========|===========|==============|============|===========================
0        |DUMP_AM           |6     |101000  |40         |38         |2             |19          |AM diagnostic read output
1        |DUMP_MM           |6     |101001  |41         |8          |1             |8           |MM diagnostic read output
2        |DUMP_PAT          |5     |10101   |21         |20         |1             |20          |PAT diagnostic read output
3        |DUMP_POST_FIFO[0] |6     |101110  |46         |19         |1             |19          |copy of tag class 0 traffic exiting FIFO
4        |DUMP_POST_FIFO[1] |6     |101111  |47         |19         |1             |19          |copy of tag class 1 traffic exiting FIFO
5        |DUMP_PRE_FIFO     |6     |101101  |45         |20         |1             |20          |copy of traffic entering FIFO
6        |DUMP_TAT[0]       |4     |1000    |8          |29         |1             |29          |TAT 0 diagnostic read output
7        |DUMP_TAT[1]       |4     |1001    |9          |29         |1             |29          |TAT 1 diagnostic read output
8        |NRNI              |2     |11      |3          |12         |1             |12          |copy of traffic exiting neuron array
9        |OVFLW[0]          |7     |1011000 |88         |1          |1             |1           |class 0 FIFO overflow warning
10       |OVFLW[1]          |7     |1011001 |89         |1          |1             |1           |class 1 FIFO overflow warning
11       |RO_ACC            |2     |01      |1          |28         |1             |28          |tag output from accumulator
12       |RO_TAT            |2     |00      |0          |32         |1             |32          |tag output from TAT
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
localparam Nbiggest_payload = 32;
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
  {{6{1'b1}}, {(34-6){1'b0}}},
  {{6{1'b1}}, {(34-6){1'b0}}},
  {{5{1'b1}}, {(34-5){1'b0}}},
  {{6{1'b1}}, {(34-6){1'b0}}},
  {{6{1'b1}}, {(34-6){1'b0}}},
  {{6{1'b1}}, {(34-6){1'b0}}},
  {{4{1'b1}}, {(34-4){1'b0}}},
  {{4{1'b1}}, {(34-4){1'b0}}},
  {{2{1'b1}}, {(34-2){1'b0}}},
  {{7{1'b1}}, {(34-7){1'b0}}},
  {{7{1'b1}}, {(34-7){1'b0}}},
  {{2{1'b1}}, {(34-2){1'b0}}},
  {{2{1'b1}}, {(34-2){1'b0}}}};

const logic [0:Nfunnel-1][NBDdata-1:0] payload_masks = ~route_masks;

const logic [0:Nfunnel-1][NBDdata-1:0] routes = '{
  'b101000  << (NBDdata - 6),
  'b101001  << (NBDdata - 6),
  'b10101   << (NBDdata - 5),
  'b101110  << (NBDdata - 6),
  'b101111  << (NBDdata - 6),
  'b101101  << (NBDdata - 6),
  'b1000    << (NBDdata - 4),
  'b1001    << (NBDdata - 4),
  'b11      << (NBDdata - 2),
  'b1011000 << (NBDdata - 7),
  'b1011001 << (NBDdata - 7),
  'b01      << (NBDdata - 2),
  'b00      << (NBDdata - 2)};

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
  if (test[0] == 1)
    leaf = DUMP_AM;
  else if (test[1] == 1)
    leaf = DUMP_MM;
  else if (test[2] == 1)
    leaf = DUMP_PAT;
  else if (test[3] == 1)
    leaf = DUMP_POST_FIFO0;
  else if (test[4] == 1)
    leaf = DUMP_POST_FIFO1;
  else if (test[5] == 1)
    leaf = DUMP_PRE_FIFO;
  else if (test[6] == 1)
    leaf = DUMP_TAT0;
  else if (test[7] == 1)
    leaf = DUMP_TAT1;
  else if (test[8] == 1)
    leaf = NRNI;
  else if (test[9] == 1)
    leaf = OVFLW0;
  else if (test[10] == 1)
    leaf = OVFLW1;
  else if (test[11] == 1)
    leaf = RO_ACC;
  else if (test[12] == 1)
    leaf = RO_TAT;
  else
    leaf = INVALID;

logic [NBDdata-1:0] masked_payload;
assign masked_payload = BD_in.d & payload_masks[leaf];
assign dec_out.payload = masked_payload[Nbiggest_payload-1:0];

assign dec_out.leaf_code = leaf; // enum -> int

assign dec_out.v = BD_in.v;
assign BD_in.a = dec_out.a;

endmodule

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
// BDHornDeserializer deserializes the two-part AMMM word
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
ChannelSplit #(.N(N), .Mask({4'b1111, 32'd0}), .Code0(DUMP_AM_code))
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
