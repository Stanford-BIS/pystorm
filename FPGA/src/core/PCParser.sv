`include "../lib/Interfaces.svh"
`include "../lib/Channel.svh"
`include "../lib/ChannelUtil.svh"

module PCParser #(
  parameter NPCin = 27,
  parameter Nconf = 16,
  parameter Nreg = 32,
  parameter Nchan = 2) (

  // output registers
  output logic [Nreg-1:0][Nconf-1:0] conf_reg_out,

  // output channels
  ChannelArray conf_channel_out, // Nchan channels, Nconf wide

  // output to BD
  UnencodedBDWordChannel BD_data_out,
  
  // input, from PC
  Channel PC_in_ext,

  // reset values for the conf registers
  // set in PCMapper (because PCMapper assigns the regs meaning)
  input [Nreg-1:0][Nconf-1:0] conf_reg_reset_vals,

  // input from TM
  input stall,

  input clk, reset);

/////////////////////////////////////////////////
// stall input when running ahead

Channel #(NPCin) PC_in();
ChannelStaller input_staller(.out(PC_in), .in(PC_in_ext), .stall(stall));


/////////////////////////////////////////////////
//
// From a software perspective, sending inputs to the FPGA is straightforward:
// config data either programs a register, or feeds a channel.
// the programming words reflect this:
//
// This is the first phase of parsing for the FPGA configuration inputs.
// The next phase takes the register and channel outputs, and assigns them
// to their configured components, combining registers or channels to achieve
// the needed width when necessary
//
//    7     20 
// [ code | data ]
//
/////////////////////////////////////////////////
// BD-bound word
//
//  MSB                   LSB
//       7             20
// [    code       | BD_data ]
//   1       6         20
// [ 0 | leaf_code | BD_data ]
//
// codes 0-34 are horn codes
// the LSBs contain the data to be passed through
//
/////////////////////////////////////////////////
// FPGA-bound reg config word
//
//  MSB                     LSB
//          7         4    16
// [      code      | X | data ]
//   1   1      5     4    16
// [ 1 | 0 | reg_id | X | data ]
//
// codes 64-95 address programmable registers
// reg_id addresses an array of 16-bit registers
//  bigger registers needed by the FPGA are composed of multiple 16-bit registers
//  smaller registers just waste bits
// the LSBs contain (up to) 16 bits of reg data
// 
// overall, there are 16*2**6 = 1024 bits of configurable data
// if more data is needed to configure FPGA, decrease the data width
//
/////////////////////////////////////////////////
// FPGA-bound channel config word
//
//  MSB                     LSB
//          7          4    16
// [       code      | X | data ]
//   1   1       5     4    16
// [ 1 | 1 | chan_id | X | data ]
//
// codes = 96-127 address programmable channels
// channel_id addresses an array of 16-bit channels
//  bigger channels are made by deserializing multiple transmissions
//  smaller channels just waste bits
// the LSBs contain (up to) 16 bits of channel data


localparam NBDbiggest_data = 20;
localparam Narray_id_max_bits = NPCin - 2 - Nconf;

// unpack PC_in.d for FPGA-bound word 
// (coincidentally the same for reg or chan)
logic FPGA_or_BD; 
logic reg_or_channel;
logic [4:0] conf_array_id;
logic [3:0] FPGA_unused;
logic [Nconf-1:0] conf_data;
assign {FPGA_or_BD, reg_or_channel, conf_array_id, FPGA_unused, conf_data} = PC_in.d;

// unpack PC_in.d for BD-bound word
logic BD_unused;
logic [5:0] leaf_code;
logic [NBDbiggest_data-1:0] BD_data;
assign {BD_unused, leaf_code, BD_data} = PC_in.d;

// determine word type, for convenience
enum {BD_WORD, REG_WORD, CHANNEL_WORD} word_type;
always_comb
  if (FPGA_or_BD == 0)
    word_type = BD_WORD;
  else
    if (reg_or_channel == 0)
      word_type = REG_WORD;
    else
      word_type = CHANNEL_WORD;

// register updates
always_ff @(posedge clk, posedge reset)
  for (int i = 0; i < Nreg; i++) 
    if (reset == 1)
      conf_reg_out[i] <= conf_reg_reset_vals[i];
    else
      if (PC_in.v == 1 && word_type == REG_WORD && i == conf_array_id[$clog2(Nreg)-1:0])
        conf_reg_out[i] <= conf_data;
      else
        conf_reg_out[i] <= conf_reg_out[i];

// conf channels
always_comb
  for (int i = 0; i < Nchan; i++) 
    if (PC_in.v == 1 && word_type == CHANNEL_WORD && i == conf_array_id[$clog2(Nchan)-1:0]) begin
      conf_channel_out.v[i] = 1;
      conf_channel_out.d[i] = conf_data;
    end
    else begin
      conf_channel_out.v[i] = 0;
      conf_channel_out.d[i] = 'X;
    end

//logic [Nchan-1:0] conf_sel;
//assign conf_sel = conf_array_id < Nchan ? 1 << conf_array_id : 0;
//generate
//assign conf_channel_out.v = conf_sel & PC_in.v & (word_type == CHANNEL_WORD);


// BD passthrough
always_comb
  if (PC_in.v == 1 && word_type == BD_WORD) begin
    BD_data_out.v = 1;
    BD_data_out.leaf_code = leaf_code;
    BD_data_out.payload = BD_data;
  end
  else begin
    BD_data_out.v = 0;
    BD_data_out.leaf_code = 'X;
    BD_data_out.payload = 'X;
  end

// handshake input
// for the channels, there's no slack here
always_comb
  if (PC_in.v == 1)
    unique case (word_type)
      BD_WORD:
        PC_in.a = BD_data_out.a;
      REG_WORD:
        PC_in.a = PC_in.v;
      CHANNEL_WORD:
        PC_in.a = conf_channel_out.a[conf_array_id[$clog2(Nchan)-1:0]];
    endcase
  else
    PC_in.a = 0;

endmodule
