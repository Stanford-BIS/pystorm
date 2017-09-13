`include "Channel.svh"
`include "Interfaces.svh"

module BDDecoder (
  DecodedBDWordChannel dec_out,
  Channel BD_in);

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

// note that we ignore the BD serialization/full data width. That's dealt with
// in software

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
