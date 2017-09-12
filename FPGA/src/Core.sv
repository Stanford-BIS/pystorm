`include "Channel.svh"
`include "Interfaces.svh"

module Core #(
  // common parameters (in/out names relative to FPGA)
  parameter NPCcode = 8,
  parameter NPCdata = 24,
  parameter NPCout = NPCcode + NPCdata,
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

localparam NBDdata_in = 34;
localparam NBDdata_out = 21;

// PCParser/configurator parameters
localparam Nconf = 16;
localparam Nreg = 32;
localparam Nchan = 2;

// SpikeGenerator additional params
localparam N_SG_tag = Ntag;
localparam N_SG_ct = Nct;

// FIFO depth
localparam FIFOdepth = 4;

// Core includes all the components that are agnostic to both BD handshaking
// and the IO mechanism (e.g. Opal Kelly USB host module or USB IP core).
//
// Arrows for Channels, lines for plain registers.
// 
//                   +----------+                                                              
//          ||||||   |          |                                            BDTagMerge_out
// PC_in ---|FIFO|-->| PCParser |     PCParser_BD_data_out      +-----------+     v    +-----------+        +--------------+ 
//  32b     ||||||   |          |-------------------------------|           |     v    |           |        |              |  ||||||           
//                   +----------+                     ||||||    |BDTagMerge |----------| BDEncoder |--------|ChannelStaller|--|FIFO|--> BD_out
//                       | | conf_regs,          +----|FIFO|--->|           |          |           |   ^    |              |  ||||||   22b     
//                       | | conf_channels       |    ||||||    +-----------+          +-----------+   ^    +--------------+              
//                       | |                     |                                                  BDEncoder_out  |    
//                       | V                     | < < SG_tags_out                                                 |    
//                   +----------+          +----------+                                                            |    
//                   |          |          |          |                                                            |    
//                   | PCMapper |--------->| SpikeGen.| (contains a memory)                                        |    
//                   |          |----------|          |                                                            |    
//                   +----------+ SG_conf, +----------+                                                            |    
//                       | | |    SG_program_mem |                                                                 |    
//                       | | |                   |                                                                 |    
//                       | | |                   |                                                                 |    
//                       | | +-------------------|--------------------------------------------------+              |    
//                       | |                     |                                                  |              |    
//                       | | TM_conf             | time_unit_pulse                                  |              |    
//                       | |   +-----------+     |                                                  |              |    
//                       | |   |           |-----+                                                  |              |   
//                       | +---|  TimeMgr  |     |                                                  |              |
//                       |     |           |-----|--------------------------------------------------|--------------+    
//                       |     +-----------+     |                                                  |   stall_dn      
//                       |          |            |                                                  |           
//                       |          |            |                                                  |       
//               SF_conf |          |      +----------+                                             |       
//                       |          |      |          |                                             |       
//                       +----------|------|SpikeFilt.| (contains a memory)                         |       
//                                  | +----|          |                                     TS_conf |       
//                                  | |    +----------+                                             |       
//                send_HB_pulse_up, | | SF_tags_ ^                                                  |       
//                    time_elapsed  | | out      |                                                  |   
//                                  | V          |                                                  |             
//                             +----------+      |                                                  |             
//                             |          |      |                                                  |             
//                             | FPGASer. |      |                                                  |             
//                             |          |      |                                                  |             
//                             +----------+      |                                                  |             
//               FPGASerializer_out |            |                                                  |                    
//                                  |            |                                                  |     BDDecoder_out
//                   +----------+   |            |   ||||||        BDTagSplit_out_tags        +-----------+   v   +-----------+ 
//           ||||||  |          |<--+            +---|FIFO|-----------------------------------|           |   v   |           |   ||||||        
// PC_out <--|FIFO|--| PCPacker |                    ||||||          +----------+             |BDTagSplit |<------| BDDecoder |<--|FIFO|-- BD_in
//  32b      ||||||  |          |<--+                                |          |<------------|           |       |           |   ||||||    34b  
//                   +----------+   |                                |  BDSer.  | BDTagSplit_ +-----------+       +-----------+  
//                                  +--------------------------------|          | out_other                                  
//                                               BDSerializer_out    +----------+
//
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
Channel #(NBDdata_out) BDEncoder_out();

// data channels: BD -> PC
DecodedBDWordChannel BDDecoder_out();
DecodedBDWordChannel BDTagSplit_out_other();
TagCtChannel #(Ntag, Nct) BDTagSplit_out_tags();
TagCtChannel #(Ntag, Nct) BDTagSplit_out_tags_post_FIFO();
SpikeFilterOutputChannel SF_tags_out();
SerializedPCWordChannel BDSerializer_out();
SerializedPCWordChannel FPGASerializer_out();

/////////////////////////////////////////////
// reset
pReset = BD_conf.pReset;
sReset = BD_conf.sReset;

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
  BDEncoder_out,
  BDTagMerge_out);

// BD_out staller
ChannelStaller BD_out_stall(
  BD_out_pre_FIFO,
  BDEncoder_out,
  stall_dn);

/////////////////////////////////////////////
// BD -> PC datapath

// BDDecoder
BDDecoder BD_decoder(BDDecoder_out, BD_in_post_FIFO);

// BDTagSplit
BDTagSplit #(
  NBDdata_in, 
  Ntag, 
  Nct) 
BD_tag_split(
  BDTagSplit_out_tags,
  BDTagSplit_out_other,
  BDDecoder_out,
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
  .NPCdata(NPCdata))
PC_packer(
  PC_out_pre_FIFO,
  BDSerializer_out,
  FPGASerializer_out,
  clk, reset);

endmodule

///////////////////////////
// TESTBENCH

module Core_tb;

parameter NPCcode = 8;
parameter NPCdata = 24;

parameter NBDdata_in = 34;
parameter NBDdata_out = 21;


// PC-side
Channel #(NPCcode + NPCdata) PC_in();
Channel #(NPCcode + NPCdata) PC_out();

// BD-side
Channel #(NBDdata_out) BD_out();
Channel #(NBDdata_in) BD_in();

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

RandomChannelSrc #(.N(NPCcode + NPCdata), .ClkDelaysMin(0), .ClkDelaysMax(4)) PC_in_src(PC_in, clk, reset);
ChannelSink #(.ClkDelaysMin(0), .ClkDelaysMax(1)) PC_out_sink(PC_out, clk, reset);

RandomChannelSrc #(.N(NBDdata_in)) BD_in_src(BD_in, clk, reset);

ChannelSink BD_out_sink(BD_out, clk, reset);

Core dut(.*);

endmodule
