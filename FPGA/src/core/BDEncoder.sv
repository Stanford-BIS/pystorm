`include "../lib/Channel.svh"
`include "../lib/ChannelUtil.svh"
`include "../lib/Interfaces.svh"
`include "Serializer.sv"

// this table is from the wiki and is wrong! routes have reversed bit-order
/*
order    |leaf name           |depth  |route          |data   |serial-|chunk  |purpose
(BDHornEP|                    |       |(bin)          |width  |ization|width  |
 code)   |                    |       |               |       |       |       |
=========|====================|=======|===============|=======|=======|=======|===============================================
0        |ADC                 |4      |1010           |3      |1      |3      |small(0)/large(1)currenttoggleforADC
1        |DAC[0]              |8      |10110000       |11     |1      |11     |DIFF_GDACbiasvalue
2        |DAC[10]             |7      |1011101        |11     |1      |11     |SOMA_INHDACbiasvalue
3        |DAC[11]             |7      |1011110        |11     |1      |11     |SYN_PUDACbiasvalue
4        |DAC[12]             |7      |1011111        |11     |1      |11     |UNUSED("ghostDAC")
5        |DAC[1]              |8      |10110001       |11     |1      |11     |DIFF_RDACbiasvalue
6        |DAC[2]              |8      |10110010       |11     |1      |11     |SOMA_OFFSETDACbiasvalue
7        |DAC[3]              |8      |10110011       |11     |1      |11     |SYN_LKDACbiasvalue
8        |DAC[4]              |8      |10110100       |11     |1      |11     |SYN_DCDACbiasvalue
9        |DAC[5]              |8      |10110101       |11     |1      |11     |SYN_PDDACbiasvalue
10       |DAC[6]              |8      |10110110       |11     |1      |11     |ADC_BIAS_2DACbiasvalue
11       |DAC[7]              |8      |10110111       |11     |1      |11     |ADC_BIAS_1DACbiasvalue
12       |DAC[8]              |8      |10111000       |11     |1      |11     |SOMA_REFDACbiasvalue
13       |DAC[9]              |8      |10111001       |11     |1      |11     |SOMA_EXCDACbiasvalue
14       |DELAY[0]            |7      |1000000        |8      |1      |8      |FIFO:DCTdelaylineconfig
15       |DELAY[1]            |7      |1000010        |8      |1      |8      |FIFO:PGdelaylineconfig
16       |DELAY[2]            |8      |10001110       |8      |1      |8      |TAT0delaylineconfig
17       |DELAY[3]            |8      |10001111       |8      |1      |8      |TAT1delaylineconfig
18       |DELAY[4]            |6      |100100         |8      |1      |8      |PATdelaylineconfig
19       |DELAY[5]            |7      |1001100        |8      |1      |8      |MMdelaylineconfig
20       |DELAY[6]            |7      |1001101        |8      |1      |8      |AMdelaylineconfig
21       |INIT_FIFO_DCT       |7      |1000110        |11     |1      |11     |insertsatagintotheDCTsideoftheFIFOwithct=1
22       |INIT_FIFO_HT        |8      |10001000       |1      |1      |1      |triggersetsFIFOhead/tailregistertoemptystate
23       |NeuronConfig        |3      |110            |18     |1      |18     |programminginputforneuronarraytileSRAM
24       |NeuronDumpToggle    |4      |1111           |2      |1      |2      |togglesdata/dumptrafficforneuronarrayoutput
25       |NeuronInject        |4      |1110           |11     |1      |11     |directspikeinjectiontoneuronarray
26       |PROG_AMMM           |6      |100111         |42     |4      |11     |AM/MMprogramming/diagnosticport
27       |PROG_PAT            |6      |100101         |27     |4      |7      |PATprogramming/diagnosticport
28       |PROG_TAT[0]         |7      |1000001        |31     |4      |8      |TAT0programming/diagnosticport
29       |PROG_TAT[1]         |7      |1000011        |31     |4      |8      |TAT1programming/diagnosticport
30       |RI                  |1      |0              |20     |1      |20     |maintaginputtoFIFO
31       |TOGGLE_POST_FIFO[0] |8      |10001010       |2      |1      |2      |togglesdata/dumptrafficforFIFOtagclass0output
32       |TOGGLE_POST_FIFO[1] |8      |10001011       |2      |1      |2      |togglesdata/dumptrafficforFIFOtagclass1output
33       |TOGGLE_PRE_FIFO     |8      |10001001       |2      |1      |2      |togglesdata/dumptrafficforFIFOinput
*/


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
// does BD 1-to-4 serialization for PROG_* leaves, as per the wiki
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
module BDFunnelSerializer (
  UnencodedBDWordChannel words_out,
  UnencodedBDWordChannel words_in,
  input clk, reset);

localparam unsigned NBDdata = 21;
localparam unsigned Nbiggest_payload = 24;
localparam unsigned Nhorn = 34;
localparam unsigned Ncode = 6;
localparam unsigned Nlongest_route = 8;

genvar i;

// these words are all serialized 1-to-2 by the PC
// we first deserialize 2-to-1, then serialize the result 1-to-4
typedef enum {AMMM=0, PAT=1, TAT0=2, TAT1=3} prog_type;
// note ascending indices! table order
const logic [0:3][Ncode-1:0] PROG_codes = '{26, 27, 28, 29};

///////////////////////////////////////////
// bools to use for the split logic

logic[0:3] PROG_test;
generate
  for (i = 0; i < 4; i++) begin : generate_PROG_test
    assign PROG_test[i] = words_in.leaf_code == PROG_codes[i] ? 1 : 0;
  end
endgenerate

logic other_test;
assign other_test = ~(|PROG_test);

///////////////////////////////////////////
// split out PROG codes and other traffic

// prog words
Channel #(Nbiggest_payload) PROG_split[0:3]();
generate 
  for (i = 0; i < 4; i++) begin : generate_PROG_split
    assign PROG_split[i].v = (words_in.v == 1 && PROG_test[i] == 1) ? 1 : 0;
    assign PROG_split[i].d = (words_in.v == 1 && PROG_test[i] == 1) ? words_in.payload : 'X;
  end
endgenerate

// other words (need to keep the code)
// this bypasses the next couple stages
UnencodedBDWordChannel other_split();
assign other_split.v         = (words_in.v == 1 && other_test == 1) ?                  1 : 0;
assign other_split.payload   = (words_in.v == 1 && other_test == 1) ?   words_in.payload : 'X;
assign other_split.leaf_code = (words_in.v == 1 && other_test == 1) ? words_in.leaf_code : 'X;

// handshake
assign words_in.a = PROG_split[AMMM].a | PROG_split[PAT].a | PROG_split[TAT0].a | PROG_split[TAT1].a | other_split.a;

///////////////////////////////////////////
// deserialize 2-to-1

Channel #(2*Nbiggest_payload) PROG_deser[0:3]();
Deserializer #(.Nin(Nbiggest_payload), .Nout(2*Nbiggest_payload)) deser[0:3](PROG_deser, PROG_split, clk, reset);

///////////////////////////////////////////
// rearrange in preparation for serizlization 
// this works a little differently for each word
// all we have to do is rearrange the bits with some padded zeros
// and feed to a deserializer
// first, rearrange words

// AMMM :  BD does : 11 - 21 - 42
//         we do   : 44 - 22 - 11
//
//                 42
//             21       21
//          11   11   11   11
//   44 = [xbbb|bbbb|xbbb|bbbb] (what we put into deser)
//   sent later <----- earlier

localparam AMMM_N = 11;
Channel #(4*AMMM_N) AMMM_rearr();
assign AMMM_rearr.d = {
  1'b0, PROG_deser[AMMM].d[3*AMMM_N-1 +: AMMM_N-1], // 0 inserted!
        PROG_deser[AMMM].d[2*AMMM_N-1 +: AMMM_N  ],
  1'b0, PROG_deser[AMMM].d[1*AMMM_N   +: AMMM_N-1], // 0 inserted!
        PROG_deser[AMMM].d[0*AMMM_N   +: AMMM_N  ]};
assign AMMM_rearr.v = PROG_deser[AMMM].v;
assign PROG_deser[AMMM].a = AMMM_rearr.a;


// PAT  :  BD does :  7 - 14 - 27
//         we do   : 28 - 14 -  7
//
//                 27 
//            14        14
//          7    7    7    7
//   28 = [xbbb|bbbb|bbbb|bbbb]

localparam PAT_N = 7;
Channel #(4*PAT_N) PAT_rearr();
assign PAT_rearr.d = {
  1'b0, PROG_deser[PAT].d[3*PAT_N +: PAT_N-1], // 0 inserted!
        PROG_deser[PAT].d[2*PAT_N +: PAT_N  ],
        PROG_deser[PAT].d[1*PAT_N +: PAT_N  ], 
        PROG_deser[PAT].d[0*PAT_N +: PAT_N  ]};
assign PAT_rearr.v = PROG_deser[PAT].v;
assign PROG_deser[PAT].a = PAT_rearr.a;


// TAT* :  BD does :  8 - 16 - 31
//         we do   : 32 - 16 -  8
//
//                 31 
//            16        16
//          8    8    8    8
//   32 = [xbbb|bbbb|bbbb|bbbb]

localparam TAT0_N = 8;
Channel #(4*TAT0_N) TAT0_rearr();
assign TAT0_rearr.d = {
  1'b0, PROG_deser[TAT0].d[3*TAT0_N-1 +: TAT0_N-1], // 0 inserted!
        PROG_deser[TAT0].d[2*TAT0_N-1 +: TAT0_N  ],
  1'b0, PROG_deser[TAT0].d[1*TAT0_N   +: TAT0_N-1], // 0 inserted!
        PROG_deser[TAT0].d[0*TAT0_N   +: TAT0_N  ]};
assign TAT0_rearr.v = PROG_deser[TAT0].v;
assign PROG_deser[TAT0].a = TAT0_rearr.a;

localparam TAT1_N = 8;
Channel #(4*TAT1_N) TAT1_rearr();
assign TAT1_rearr.d = {
  1'b0, PROG_deser[TAT1].d[3*TAT1_N-1 +: TAT1_N-1], // 0 inserted!
        PROG_deser[TAT1].d[2*TAT1_N-1 +: TAT1_N  ],
  1'b0, PROG_deser[TAT1].d[1*TAT1_N   +: TAT1_N-1], // 0 inserted!
        PROG_deser[TAT1].d[0*TAT1_N   +: TAT1_N  ]};
assign TAT1_rearr.v = PROG_deser[TAT1].v;
assign PROG_deser[TAT1].a = TAT1_rearr.a;


///////////////////////////////////////////
// serialize rearranged words 1-to-4

Channel #(Nbiggest_payload) PROG_ser[0:3](); // ser should just zero-extend narrower outputs
Serializer #(.Nin(4*AMMM_N), .Nout(AMMM_N)) AMMM_ser(PROG_ser[AMMM], AMMM_rearr, clk, reset);
Serializer #(.Nin(4* PAT_N), .Nout( PAT_N))  PAT_ser(PROG_ser[PAT ],  PAT_rearr, clk, reset);
Serializer #(.Nin(4*TAT0_N), .Nout(TAT0_N)) TAT0_ser(PROG_ser[TAT0], TAT0_rearr, clk, reset);
Serializer #(.Nin(4*TAT1_N), .Nout(TAT1_N)) TAT1_ser(PROG_ser[TAT1], TAT1_rearr, clk, reset);

// turn into UnencodedBDWordChannel
UnencodedBDWordChannel PROG_ser_coded[0:3]();
generate
  for (i = 0; i < 4; i++) begin : generate_PROG_ser_coded
    assign PROG_ser_coded[i].leaf_code = PROG_codes[i];
    assign PROG_ser_coded[i].payload   = PROG_ser[i].d;
    assign PROG_ser_coded[i].v         = PROG_ser[i].v;
    assign PROG_ser[i].a               = PROG_ser_coded[i].a;
  end
endgenerate

///////////////////////////////////////////
// merge serialized streams

// pack merge_in[]
Channel #(Nbiggest_payload + Ncode) merge_in[4:0]();

generate
  for(i = 0; i < 4; i++) begin : generate_merge_in
    assign merge_in[i].d       = {PROG_ser_coded[i].leaf_code, PROG_ser_coded[i].payload};
    assign merge_in[i].v       = PROG_ser_coded[i].v;
    assign PROG_ser_coded[i].a = merge_in[i].a;
  end
endgenerate

assign merge_in[4].d = {other_split.leaf_code, other_split.payload};
assign merge_in[4].v = other_split.v;
assign other_split.a = merge_in[4].a;

// do merge
//
//  0                        
// ---|\  01                 
//  1 | |--+                 
// ---|/   |                 
//         +--|\  0123       
//            | |-----|\  top
//  2      +--|/      | |----
// ---|\   |       +--|/     
//  3 | |--+       |         
// ---|/  23       |         
//                 |         
//  4              |         
// ----------------+         
//

Channel #(Nbiggest_payload + Ncode) merge_01_out();
Channel #(Nbiggest_payload + Ncode) merge_23_out();
Channel #(Nbiggest_payload + Ncode) merge_0123_out();
Channel #(Nbiggest_payload + Ncode) merge_out();
ChannelMerge merge_01(merge_01_out, merge_in[0], merge_in[1], clk, reset);
ChannelMerge merge_23(merge_23_out, merge_in[2], merge_in[3], clk, reset);
ChannelMerge merge_0123(merge_0123_out, merge_01_out, merge_23_out, clk, reset);
ChannelMerge merge_top(merge_out, merge_0123_out, merge_in[4], clk, reset);

// put a FIFO in, that's a lot of stages of logic we just went through 
// (merges don't cut combinational paths, as designed);
Channel #(Nbiggest_payload + Ncode) merge_out_post_FIFO();
localparam FIFOdepth = 2;
ChannelFIFO #(.N(Nbiggest_payload + Ncode), .D(FIFOdepth)) output_FIFO(merge_out_post_FIFO, merge_out, clk, reset);

// unpack merge_out
assign {words_out.leaf_code, words_out.payload} = merge_out_post_FIFO.d;
assign words_out.v                              = merge_out_post_FIFO.v;
assign merge_out_post_FIFO.a                    = words_out.a;

endmodule


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
// BDFunnelEncoder does software BDHornEP codes -> BD horn encoding, as per the wiki
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
module BDFunnelEncoder (
  Channel BD_data_out,
  UnencodedBDWordChannel words_in);

// BD input word format:
// [ X | payload | route ]

localparam unsigned NBDdata = 21;
localparam unsigned Nhorn = 34;
localparam unsigned Ncode = 6;
localparam unsigned Nlongest_route = 8;

// note ascending outer indices! table order
// copied from above, with extra zeros filled in, then flipped (wiki table is backwards)
const logic [0:Nhorn-1][Nlongest_route-1:0] routes = '{
  'b00000101,
  'b00001101,
  'b01011101,
  'b00111101,
  'b01111101,
  'b10001101,
  'b01001101,
  'b11001101,
  'b00101101,
  'b10101101,
  'b01101101,
  'b11101101,
  'b00011101,
  'b10011101,
  'b00000001,
  'b00100001,
  'b01110001,
  'b11110001,
  'b00001001,
  'b00011001,
  'b01011001,
  'b00110001,
  'b00010001,
  'b00000011,
  'b00001111,
  'b00000111,
  'b00111001,
  'b00101001,
  'b01000001,
  'b01100001,
  'b00000000,
  'b01010001,
  'b11010001,
  'b10010001};

const logic [0:Nhorn-1][Ncode-1:0] route_lens = '{
  4,
  8,
  7,
  7,
  7,
  8,
  8,
  8,
  8,
  8,
  8,
  8,
  8,
  8,
  7,
  7,
  8,
  8,
  6,
  7,
  7,
  7,
  8,
  3,
  4,
  4,
  6,
  6,
  7,
  7,
  1,
  8,
  8,
  8};

///////////////////////////////////////////
// logic

logic [NBDdata-1:0] data_shifted1;
logic [NBDdata-1:0] data_shifted3;
logic [NBDdata-1:0] data_shifted4;
logic [NBDdata-1:0] data_shifted6;
logic [NBDdata-1:0] data_shifted7;
logic [NBDdata-1:0] data_shifted8;
assign data_shifted1 = words_in.payload << 1;
assign data_shifted3 = words_in.payload << 3;
assign data_shifted4 = words_in.payload << 4;
assign data_shifted6 = words_in.payload << 6;
assign data_shifted7 = words_in.payload << 7;
assign data_shifted8 = words_in.payload << 8;

logic [NBDdata-1:0] route_sel;
assign route_sel = routes[words_in.leaf_code];

logic [Ncode-1:0] route_len;
assign route_len = route_lens[words_in.leaf_code];

always_comb
  if (words_in.leaf_code < Nhorn) begin
    BD_data_out.v = words_in.v;
    case (route_len)
      1: 
        BD_data_out.d = data_shifted1 | route_sel;
      3:
        BD_data_out.d = data_shifted3 | route_sel;
      4:
        BD_data_out.d = data_shifted4 | route_sel;
      6:
        BD_data_out.d = data_shifted6 | route_sel;
      7:
        BD_data_out.d = data_shifted7 | route_sel;
      8:
        BD_data_out.d = data_shifted8 | route_sel;
      default:
        BD_data_out.d = data_shifted1 | route_sel;
    endcase
  end
  else begin
    BD_data_out.v = 0;
    BD_data_out.d = 'X;
  end

// handshake bad inputs
always_comb
  if (words_in.leaf_code < Nhorn)
    words_in.a = BD_data_out.a;
  else
    if (words_in.v == 1)
      words_in.a = 1;
    else
      words_in.a = 0;

endmodule

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
// BDEncoder combines BDSerializer and BDFunnelEncoder
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
module BDEncoder (
  Channel BD_data_out,
  UnencodedBDWordChannel words_in, 
  input clk, reset);

UnencodedBDWordChannel ser_out();
BDFunnelSerializer serializer(ser_out, words_in, clk, reset);
BDFunnelEncoder funnel_enc(BD_data_out, ser_out);

endmodule
