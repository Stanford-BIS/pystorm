`include "Channel.svh"

module PCParser #(
  parameter NPCin = 24,
  parameter NBDdata = 21,
  parameter Nconf = 16,
  parameter Nreg = 32,
  parameter Nchan = 8) (

  // output registers
  output logic [Nreg-1:0][Nconf-1:0] conf_reg_out,

  // output channels
  ChannelArray conf_channel_out, // Nchan channels, Nconf wide

  // passthrough output to BD
  Channel BD_data_out,
  
  // input, from PC
  Channel PC_in,

  input [Nreg-1:0][Nconf-1:0] conf_reg_reset_vals,

  input clk, 
  input reset);

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
/////////////////////////////////////////////////
// BD-bound word
//
//   1   2      21
// [ 0 | X | BD_data ]
//
// MSB = 0 means BD passthrough
// the LSBs contain the data to be passed through
//
/////////////////////////////////////////////////
// FPGA-bound reg config word
//
//   1   1     6       16
// [ 1 | 0 | reg_id | data ]
//
// MSB = 1 means FPGA config word
// MSB-1 = 0 is means register programming
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
//   1   1        6        16
// [ 1 | 1 | channel_id | data ]
//
// MSB = 1 means FPGA config word
// MSB-1 = 1 is means channel programming
// channel_id addresses an array of 16-bit channels
//  bigger channels are made by deserializing multiple transmissions
//  smaller channels just waste bits
// the LSBs contain (up to) 16 bits of channel data

parameter Narray_id_max_bits = NPCin - 2 - Nconf;

// unpack PC_in.d
logic FPGA_or_BD;

logic BD_data;

logic reg_or_channel;
logic [Narray_id_max_bits-1:0] conf_array_id;
logic [Nconf-1:0] conf_data;

assign {FPGA_or_BD, reg_or_channel, conf_array_id, conf_data} = PC_in.d;
assign BD_data = PC_in.d[NBDdata-1:0];

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
    BD_data_out.d = BD_data;
  end
  else begin
    BD_data_out.v = 0;
    BD_data_out.d = 'X;
  end

// handshake input
// for the channels, there's no slack here
always_comb
  unique case (word_type)
    BD_WORD:
      PC_in.a = BD_data_out.a;
    REG_WORD:
      PC_in.a = PC_in.v;
    CHANNEL_WORD:
      PC_in.a = conf_channel_out.a[conf_array_id[$clog2(Nchan)-1:0]];
  endcase

endmodule

///////////////////////////
// TESTBENCH
module PCParser_tb;

parameter NPCin = 24;
parameter NBDdata = 21;
parameter Nconf = 16;
parameter Nreg = 4;
parameter Nchan = Nreg;

// output registers
logic [Nreg-1:0][Nconf-1:0] conf_reg_out;

// output channels
ChannelArray #(Nconf, Nchan) conf_channel_out(); 
Channel #(Nconf) conf_channel_out_unpacked[Nchan-1:0](); 
UnpackChannelArray #(Nchan) conf_unpacker(conf_channel_out, conf_channel_out_unpacked);

// passthrough output to BD
Channel #(NBDdata) BD_data_out();

// input; from PC
Channel #(NPCin) PC_in();

logic [Nreg-1:0][Nconf-1:0] conf_reg_reset_vals = '0;

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

// PC sender
RandomChannelSrc #(.N(NPCin)) PC_src(PC_in, clk, reset);

// BD receiver
ChannelSink BD_sink(BD_data_out, clk, reset);

// conf_channel receivers
ChannelSink chan_sinks[Nchan-1:0](conf_channel_out_unpacked, clk, reset);

PCParser #(NPCin, NBDdata, Nconf, Nreg, Nchan) dut(.*);

endmodule

