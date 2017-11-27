`include "../lib/Channel.svh"
`include "../lib/ChannelUtil.svh"
`include "../lib/Interfaces.svh"
`include "BDEncoder.sv"
`include "BDSerializer.sv"
`include "BDTagSplit.sv"
`include "PCMapper.sv"
`include "PCParser.sv"
`include "SpikeGeneratorArray.sv"
`include "BDDecoder.sv"
`include "BDTagMerge.sv"
`include "FPGASerializer.sv"
`include "PCPacker.sv"
`include "SpikeFilterArray.sv"
`include "TimeMgr.sv"
`include "GlobalTagParser.sv"

module Core #(
  // common parameters (in/out names relative to FPGA)
  parameter NPCcode = 8,
  parameter NPCdata = 24,
  parameter NPCroute = 10,
  parameter NPCout = NPCcode + NPCdata + NPCroute,
  parameter NPCin = NPCcode + NPCdata,

  // parameters for SpikeFilterArray
  parameter N_SF_filts = 10,
  parameter N_SF_state = 27,
  parameter N_SF_ct = 10,

  // parameters for SpikeGeneratorArray
  parameter N_SG_gens = 8,
  parameter N_SG_period = 16,

  // parameters for TimeMgr
  parameter N_TM_time = 48,
  parameter N_TM_unit = 16) 
(
  // PC-side
  Channel PC_out,
  Channel PC_in,

  // BD-side
  Channel BD_out,
  Channel BD_in,

  output logic pReset, 
  output logic sReset,

  input adc0,
  input adc1,

  input clk, reset);

// local parameters, unmodifiable without changing submodules

// common parameters (in/out names relative to FPGA)
localparam Ntag = 11;
localparam Nct = 9;
localparam Nglobal = 12;

localparam NBDdata_in = 34;
localparam NBDdata_out = 21;

// PCParser/configurator parameters
localparam Nconf = 16;
localparam Nreg = 64;
localparam Nchan = 2;

// SpikeGenerator additional params
localparam N_SG_tag = Ntag;
localparam N_SG_ct = Nct;

// FIFO depth
localparam FIFOdepth = 4;

// **THIS NEEDS TO BE EDITED TO ADD GLOBAL ROUTE STUFF**
//s
// Core includes all the components that are agnostic to both BD handshaking
// and the IO mechanism (e.g. Opal Kelly USB host module or USB IP core).
//
// Arrows for Channels, lines for plain registers.
// 
//                   +----------+                                                       
//          ||||||   |          |                                                           BDTagMerge_out
// PC_in ---|FIFO|-->| PCParser |          PCParser_BD_data_out                  +-----------+  v   +-----------+     
//  32b     ||||||   |          |------------------------------------------------|           |  v   |           |     ||||||    
//                   +----------+                                  ||||||        |BDTagMerge |------| BDEncoder |-----|FIFO|--> BD_out
//                     | | |                                  +----|FIFO|------->|           |      |           |     ||||||   22b 
//                +----+ | | conf_channel                     |    ||||||        +-----------+      +-----------+                 
//                |      | | conf_regs,                       |                                                                   
//                |      | V          +-----------------------|---------------------------------------------------------------> pReset, sReset
//                |  +----------+     |                       |                                   BD_conf       
//                |  |          |-----+    +----------+       |                                                               
//                |  | PCMapper |--------->|          |-------+                                             
//                |  |          |----------| SpikeGen.|    SG_tags_out                                                     
//                |  +----------+    ^     |          |                                                          
//       stall_dn |      | | |    SG_conf, +----------+                                                                   
//                |      | | |    SG_program_mem |                                                               
//                |      | | |                   |                                                               
//                |      | | +-------------------|-------------------------------------------+                   
//                |      | |                     |                                           |                   
//                |      | | TM_conf             |                                           |                   
//                |      | |   +-----------+     |                                           |                   
//                |      | +---|           |     |                                           |                  
//                |      |     |  TimeMgr  |-----+  time_unit_pulse                          |               
//                +------|-----|           |     |                                           |                   
//                       |     +-----------+     |                                           |               
//                       |          |            |                                           |           
//                       |          |            |                                           |       
//               SF_conf |          |      +----------+                                      |       
//                       |          |      |          |                                      |       
//                       +----------|------|SpikeFilt.|                                      |       
//                                  | +----|          |                              TS_conf |       
//                                  | |    +----------+                                      |       
//                send_HB_pulse_up, | | SF_tags_ ^                                           |       
//                    time_elapsed  | | out      |                                           |   
//                                  | V          |                                           |             
//                             +----------+      |                                           |             
//                             |          |      |                                           |             
//                             | FPGASer. |      |                                           |             
//                             |          |      |                                           |                          X <------ ADC
//                             +----------+      |                                           |             
//               FPGASerializer_out |            |                                           |                    
//                                  |            |                                           |          BDDecoder_out
//                   +----------+   |            |   ||||||     BDTagSplit_out_tags    +-----------+           v  +-----------+ 
//           ||||||  |          |<--+            +---|FIFO|----------------------------|           |   ||||||  v  |           |   ||||||
// PC_out <--|FIFO|--| PCPacker |                    ||||||          +----------+      |BDTagSplit |<--|FIFO|-----| BDDecoder |<--|FIFO|-- BD_in
//  32b      ||||||  |          |<--+                                |          |<-----|           |   ||||||     |           |   ||||||    34b
//                   +----------+   |                                |  BDSer.  |   ^  +-----------+              +-----------+  
//                                  +--------------------------------|          |   ^                    
//                                               BDSerializer_out    +----------+  BDTagSplit_  
//                                                                                 out_other  
//


/////////////////////////////////////////////
// PCMapper signals, FPGA config data

// IO FIFO signals
Channel #(NPCin) PC_in_post_FIFO();
Channel #(NBDdata_out) BD_out_pre_FIFO();
Channel #(NPCout) PC_out_pre_FIFO();
Channel #(NBDdata_in) BD_in_post_FIFO();

// between PCParser and mapper
logic [Nreg-1:0][Nconf-1:0] conf_regs;
logic [Nreg-1:0][Nconf-1:0] conf_reg_reset_vals;
ChannelArray #(Nconf, Nchan) conf_channels(); 

// PCMapper outputs, internal config data
// conf registers 
SpikeFilterConf #(N_SF_filts, N_SF_state) SF_conf();
SpikeGeneratorConf #(N_SG_gens) SG_conf();
TimeMgrConf #(N_TM_unit, N_TM_time) TM_conf();
TagSplitConf TS_conf();
BDIOConf BD_conf();
// conf channels
SpikeGeneratorProgChannel #(N_SG_gens, N_SG_period, N_SG_tag) SG_program_mem();

// time-related signals
logic time_unit_pulse;
logic send_HB_up_pulse;
logic stall_dn;
logic [N_TM_time-1:0] time_elapsed;

// data channels: PC -> BD
UnencodedBDWordChannel PCParser_BD_data_out();
TagCtChannel #(Ntag, Nct) SG_tags_out();
TagCtChannel #(Ntag, Nct) SG_tags_out_post_FIFO();
UnencodedBDWordChannel BDTagMerge_out();

// data channels: BD -> PC
DecodedBDWordChannel BDDecoder_out();
DecodedBDWordChannel BDDecoder_out_post_FIFO();
DecodedBDWordChannel BDTagSplit_out_other();
GlobalTagCtChannel  #(Nglobal, Ntag, Nct) BDTagSplit_out_global();
TagCtChannel #(Ntag, Nct) BDTagSplit_out_tags();
TagCtChannel #(Ntag, Nct) BDTagSplit_out_tags_post_FIFO();
SpikeFilterOutputChannel SF_tags_out();
SerializedPCWordChannel BDSerializer_out();
SerializedPCWordChannel FPGASerializer_out();
SerializedPCWordChannelwithRoute Global_tag_parser_out();

/////////////////////////////////////////////
// reset
assign pReset = BD_conf.pReset;
assign sReset = BD_conf.sReset;

/////////////////////////////////////////////
// IO FIFOs
ChannelFIFO #(.D(FIFOdepth), .N(NPCin))       PC_in_FIFO (PC_in_post_FIFO, PC_in,           clk, reset);
ChannelFIFO #(.D(FIFOdepth), .N(NPCout))      PC_out_FIFO(PC_out,          PC_out_pre_FIFO, clk, reset);
ChannelFIFO #(.D(FIFOdepth), .N(NBDdata_out)) BD_out_FIFO(BD_out,          BD_out_pre_FIFO, clk, reset);
ChannelFIFO #(.D(FIFOdepth), .N(NBDdata_in))  BD_in_FIFO (BD_in_post_FIFO, BD_in,           clk, reset);

/////////////////////////////////////////////
// Config/FPGA state modules

PCParser #(
  .NPCin(NPCin),
  .Nconf(Nconf),
  .Nreg(Nreg),
  .Nchan(Nchan)) 
PC_parser(
  conf_regs,
  conf_channels,
  PCParser_BD_data_out,
  PC_in_post_FIFO,
  conf_reg_reset_vals,
  stall_dn,
  clk, reset);

// PCMapper
PCMapper #(
  .Nconf(Nconf),
  .Nreg(Nreg),
  .Nchan(Nchan),
  .N_SF_filts(N_SF_filts),
  .N_SF_state(N_SF_state),
  .N_SF_ct(N_SF_ct),
  .N_SG_gens(N_SG_gens),
  .N_SG_period(N_SG_period),
  .N_SG_tag(N_SG_tag),
  .N_TM_time(N_TM_time),
  .N_TM_unit(N_TM_unit))
PC_mapper(
  conf_reg_reset_vals,
  SF_conf,
  SG_program_mem,
  SG_conf,
  TM_conf,
  TS_conf,
  BD_conf,
  conf_regs,
  conf_channels,
  clk, reset);

// TimeMgr
TimeMgr #(
  .Nunit(N_TM_unit),
  .Ntime(N_TM_time))
time_mgr(
  time_unit_pulse,
  send_HB_up_pulse,
  time_elapsed,
  stall_dn,
  TM_conf,
  clk, reset);

/////////////////////////////////////////////
// PC -> BD datapath

// SpikeGenerator
SpikeGeneratorArray #(
  .Ngens(N_SG_gens),
  .Nperiod(N_SG_period),
  .Ntag(N_SG_tag),
  .Nct(N_SG_ct))
SG_array(
  SG_tags_out,
  time_unit_pulse,
  SG_conf,
  SG_program_mem,
  clk, reset);

// FIFO
Channel #(.N(Ntag + Nct)) SG_tags_out_flat();
assign SG_tags_out_flat.d = {SG_tags_out.tag, SG_tags_out.ct};
assign SG_tags_out_flat.v = SG_tags_out.v;
assign SG_tags_out.a = SG_tags_out_flat.a;

Channel #(.N(Ntag + Nct)) SG_tags_out_post_FIFO_flat();
assign {SG_tags_out_post_FIFO.tag, SG_tags_out_post_FIFO.ct} = SG_tags_out_post_FIFO_flat.d;
assign SG_tags_out_post_FIFO.v = SG_tags_out_post_FIFO_flat.v;
assign SG_tags_out_post_FIFO_flat.a = SG_tags_out_post_FIFO.a;

ChannelFIFO #(.D(FIFOdepth), .N(Ntag + Nct)) SG_tags_out_FIFO(SG_tags_out_post_FIFO_flat, SG_tags_out_flat, clk, reset);

// PCParser/SG merge
BDTagMerge tag_merge(
  BDTagMerge_out,
  PCParser_BD_data_out,
  SG_tags_out_post_FIFO,
  clk, reset);

// BDEncoder
BDEncoder BD_encoder(
  BD_out_pre_FIFO,
  BDTagMerge_out,
  clk, reset);

/////////////////////////////////////////////
// BD -> PC datapath

// BDDecoder
BDDecoder BD_decoder(
  BDDecoder_out, 
  BD_in_post_FIFO,
  clk, reset);

// FIFO
Channel #(42) BDDecoder_out_flat();
Channel #(42) BDDecoder_out_post_FIFO_flat();
assign BDDecoder_out_flat.v = BDDecoder_out.v;
assign BDDecoder_out_flat.d = {BDDecoder_out.leaf_code, BDDecoder_out.payload};
assign BDDecoder_out.a = BDDecoder_out_flat.a;
ChannelFIFO #(.D(FIFOdepth), .N(42)) BDDecoder_out_FIFO(BDDecoder_out_post_FIFO_flat, BDDecoder_out_flat, clk, reset);
assign BDDecoder_out_post_FIFO.v = BDDecoder_out_post_FIFO_flat.v;
assign {BDDecoder_out_post_FIFO.leaf_code, BDDecoder_out_post_FIFO.payload} = BDDecoder_out_post_FIFO_flat.d;
assign BDDecoder_out_post_FIFO_flat.a = BDDecoder_out_post_FIFO.a;

// BDTagSplit
BDTagSplit #(
  NBDdata_in, 
  Nglobal,
  Ntag, 
  Nct) 
BD_tag_split(
  BDTagSplit_out_tags,
  BDTagSplit_out_global,
  BDTagSplit_out_other,
  BDDecoder_out_post_FIFO,
  TS_conf,
  clk, reset);

// BDSerializer
BDSerializer #(
  .Ncode(NPCcode),
  .Ndata_out(NPCdata))
BD_serializer(
  BDSerializer_out,
  BDTagSplit_out_other,
  clk, reset);

// GlobalTagParser
GlobalTagParser #(
  NBDdata_in, 
  Nglobal,
  Ntag, 
  Nct,
  NPCcode,
  NPCdata,
  NPCroute
  )
Global_tag_parser(
  BDTagSplit_out_global,
  Global_tag_parser_out,
  clk, reset);

// FIFO
Channel #(.N(Ntag + Nct)) BDTagSplit_out_tags_flat();
assign BDTagSplit_out_tags_flat.d = {BDTagSplit_out_tags.tag, BDTagSplit_out_tags.ct};
assign BDTagSplit_out_tags_flat.v = BDTagSplit_out_tags.v;
assign BDTagSplit_out_tags.a = BDTagSplit_out_tags_flat.a;

Channel #(.N(Ntag + Nct)) BDTagSplit_out_tags_post_FIFO_flat();
assign {BDTagSplit_out_tags_post_FIFO.tag, BDTagSplit_out_tags_post_FIFO.ct} = BDTagSplit_out_tags_post_FIFO_flat.d;
assign BDTagSplit_out_tags_post_FIFO.v = BDTagSplit_out_tags_post_FIFO_flat.v;
assign BDTagSplit_out_tags_post_FIFO_flat.a = BDTagSplit_out_tags_post_FIFO.a;

ChannelFIFO #(.D(FIFOdepth), .N(Ntag + Nct)) BDTagSplit_out_FIFO(BDTagSplit_out_tags_post_FIFO_flat, BDTagSplit_out_tags_flat, clk, reset);

// SpikeFilter
SpikeFilterArray #(
  .Nfilts(N_SF_filts),
  .Nstate(N_SF_state),
  .Nct(N_SF_ct))
SF_array(
  SF_tags_out,
  BDTagSplit_out_tags_post_FIFO,
  time_unit_pulse,
  SF_conf,
  clk, reset);

// FPGASerializer
FPGASerializer #(
  .NPCcode(NPCcode),
  .NPCdata(NPCdata),
  .Ntime(N_TM_time),
  .N_SF_filts(N_SF_filts),
  .N_SF_state(N_SF_state)) 
FPGA_serializer(
  FPGASerializer_out,
  send_HB_up_pulse,
  time_elapsed,
  SF_tags_out,
  clk, reset);

// PCPacker
PCPacker #(
  .NPCcode(NPCcode),
  .NPCdata(NPCdata),
  .NPCroute(NPCroute))
PC_packer(
  PC_out_pre_FIFO,
  BDSerializer_out,
  FPGASerializer_out,
  Global_tag_parser_out,
  clk, reset);

endmodule
