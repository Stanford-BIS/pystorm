`include "Channel.svh"
`include "Interfaces.svh"

module BDEncoder (
  Channel BD_out,
  UnencodedBDWordChannel enc_in);

// this table is wrong! routes have reversed bit-order
/*
leaf name           |depth  |route          |data   |serial-|chunk  |purpose
                    |       |(bin)          |width  |ization|width  |
====================|=======|===============|=======|=======|=======|===============================================
ADC                 |4      |1010           |3      |1      |3      |small(0)/large(1)currenttoggleforADC
DAC[0]              |8      |10110000       |11     |1      |11     |DIFF_GDACbiasvalue
DAC[10]             |7      |1011101        |11     |1      |11     |SOMA_INHDACbiasvalue
DAC[11]             |7      |1011110        |11     |1      |11     |SYN_PUDACbiasvalue
DAC[12]             |7      |1011111        |11     |1      |11     |UNUSED("ghostDAC")
DAC[1]              |8      |10110001       |11     |1      |11     |DIFF_RDACbiasvalue
DAC[2]              |8      |10110010       |11     |1      |11     |SOMA_OFFSETDACbiasvalue
DAC[3]              |8      |10110011       |11     |1      |11     |SYN_LKDACbiasvalue
DAC[4]              |8      |10110100       |11     |1      |11     |SYN_DCDACbiasvalue
DAC[5]              |8      |10110101       |11     |1      |11     |SYN_PDDACbiasvalue
DAC[6]              |8      |10110110       |11     |1      |11     |ADC_BIAS_2DACbiasvalue
DAC[7]              |8      |10110111       |11     |1      |11     |ADC_BIAS_1DACbiasvalue
DAC[8]              |8      |10111000       |11     |1      |11     |SOMA_REFDACbiasvalue
DAC[9]              |8      |10111001       |11     |1      |11     |SOMA_EXCDACbiasvalue
DELAY[0]            |7      |1000000        |8      |1      |8      |FIFO:DCTdelaylineconfig
DELAY[1]            |7      |1000010        |8      |1      |8      |FIFO:PGdelaylineconfig
DELAY[2]            |8      |10001110       |8      |1      |8      |TAT0delaylineconfig
DELAY[3]            |8      |10001111       |8      |1      |8      |TAT1delaylineconfig
DELAY[4]            |6      |100100         |8      |1      |8      |PATdelaylineconfig
DELAY[5]            |7      |1001100        |8      |1      |8      |MMdelaylineconfig
DELAY[6]            |7      |1001101        |8      |1      |8      |AMdelaylineconfig
INIT_FIFO_DCT       |7      |1000110        |11     |1      |11     |insertsatagintotheDCTsideoftheFIFOwithct=1
INIT_FIFO_HT        |8      |10001000       |1      |1      |1      |triggersetsFIFOhead/tailregistertoemptystate
NeuronConfig        |3      |110            |18     |1      |18     |programminginputforneuronarraytileSRAM
NeuronDumpToggle    |4      |1111           |2      |1      |2      |togglesdata/dumptrafficforneuronarrayoutput
NeuronInject        |4      |1110           |11     |1      |11     |directspikeinjectiontoneuronarray
PROG_AMMM           |6      |100111         |42     |4      |11     |AM/MMprogramming/diagnosticport
PROG_PAT            |6      |100101         |27     |4      |7      |PATprogramming/diagnosticport
PROG_TAT[0]         |7      |1000001        |31     |4      |8      |TAT0programming/diagnosticport
PROG_TAT[1]         |7      |1000011        |31     |4      |8      |TAT1programming/diagnosticport
RI                  |1      |0              |20     |1      |20     |maintaginputtoFIFO
TOGGLE_POST_FIFO[0] |8      |10001010       |2      |1      |2      |togglesdata/dumptrafficforFIFOtagclass0output
TOGGLE_POST_FIFO[1] |8      |10001011       |2      |1      |2      |togglesdata/dumptrafficforFIFOtagclass1output
TOGGLE_PRE_FIFO     |8      |10001001       |2      |1      |2      |togglesdata/dumptrafficforFIFOinput
*/

// note that we ignore the BD serialization/full data width. That's dealt with
// in software

// BD input word format:
// [ X | payload | route ]

localparam unsigned NBDdata = 21;
localparam unsigned Nbiggest_payload = 20;
localparam unsigned Nhorn = 34;
localparam unsigned Ncode = 6;
localparam unsigned Nlongest_route = 8;

///////////////////////////////////////////
// reinterpretation of table data

typedef enum {
  ADC              ,  
  DAC0             ,
  DAC10            ,  
  DAC11            ,  
  DAC12            ,  
  DAC1             ,
  DAC2             ,
  DAC3             ,
  DAC4             ,
  DAC5             ,
  DAC6             ,
  DAC7             ,
  DAC8             ,
  DAC9             ,
  DELAY0           ,
  DELAY1           ,
  DELAY2           ,
  DELAY3           ,
  DELAY4           ,
  DELAY5           ,
  DELAY6           ,
  INIT_FIFO_DCT    ,  
  INIT_FIFO_HT     ,  
  NEURONCONFIG     ,  
  NEURONDUMPTOGGLE ,  
  NEURONINJECT     ,  
  PROG_AMMM        ,  
  PROG_PAT         ,  
  PROG_TAT0        ,
  PROG_TAT1        ,
  RI               ,  
  TOGGLE_POST_FIFO0,
  TOGGLE_POST_FIFO1,
  TOGGLE_PRE_FIFO  ,
  INVALID} leaf_enum;

// XXX not used
//leaf_enum leaf;

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

const logic [Nhorn-1:0][Ncode-1:0] route_lens = '{
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
assign data_shifted1 = enc_in.payload << 1;
assign data_shifted3 = enc_in.payload << 3;
assign data_shifted4 = enc_in.payload << 4;
assign data_shifted6 = enc_in.payload << 6;
assign data_shifted7 = enc_in.payload << 7;
assign data_shifted8 = enc_in.payload << 8;

logic [NBDdata-1:0] route_sel;
assign route_sel = routes[enc_in.leaf_code];

logic [Ncode-1:0] route_len;
assign route_len = route_lens[enc_in.leaf_code];

always_comb
  if (enc_in.leaf_code < Nhorn) begin
    BD_out.v = enc_in.v;
    case (route_len)
      1: 
        BD_out.d = data_shifted1 | route_sel;
      3:
        BD_out.d = data_shifted3 | route_sel;
      4:
        BD_out.d = data_shifted4 | route_sel;
      6:
        BD_out.d = data_shifted6 | route_sel;
      7:
        BD_out.d = data_shifted7 | route_sel;
      8:
        BD_out.d = data_shifted8 | route_sel;
      default:
        BD_out.d = data_shifted1 | route_sel;
    endcase
  end
  else begin
    BD_out.v = 0;
    BD_out.d = 'X;
  end

// handshake bad inputs
always_comb
  if (enc_in.leaf_code < Nhorn)
    enc_in.a = BD_out.a;
  else
    if (enc_in.v == 1)
      enc_in.a = 1;
    else
      enc_in.a = 0;

endmodule
