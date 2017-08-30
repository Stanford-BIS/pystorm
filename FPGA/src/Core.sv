`include "Channel.svh"
`include "Interfaces.svh"

module Core #(
  // common parameters (in/out names relative to FPGA)
  parameter NPCout = 40,
  parameter NPCin = 24,

  parameter NBDdata_in = 34,
  parameter NBDdata_out = 22,

  parameter Ntag = 11,
  parameter Nct = 9,

  // PCParser/configurator parameters
  parameter Nconf = 16,
  parameter Nreg = 32,
  parameter Nchan = 2,

  // parameters for SpikeFilterArray
  parameter N_SF_filts = 10,
  parameter N_SF_state = 27,
  parameter N_SF_ct = 10,

  // parameters for SpikeGeneratorArray
  parameter N_SG_gens = 8,
  parameter N_SG_period = 16,
  parameter N_SG_tag = Ntag,
  parameter N_SG_ct = Nct,

  // paramters for TimeMgr
  parameter N_TM_time = 48,
  parameter N_TM_unit = 16) (

  // PC-side
  Channel PC_in,
  Channel PC_out,

  // BD-side
  Channel BD_out,
  Channel BD_in,

  input clk, reset);

// Core includes all the components that are agnostic to both BD handshaking
// and the IO mechanism (e.g. Opal Kelly USB host module or USB IP core).
//
// Arrows for Channels, lines for plain registers.
// Bigger boxes are more complicated modules.
// 
//            +----------+                                                                    
//            |          |                                         (module ChannelSplit)
// PC_in ---->| PCParser |        PCParser_BD_data_out                      PCParser_SG_merged          
//  24b       |          |------------------------------------------------->|\     +--------------+               
//            +----------+                             +-----------+        | |----|ChannelStaller|----> BD_out
//                | | conf_regs,          +----------->|TagCtPacker|------->|/     +--------------+       22b     
//                | | conf_channels       |            +-----------+ SG_BD_data_out        |    
//                | V                     | SG_tags_out                                    |    
//            +----------+          +----------+                                           |    
//            |          |          |          |                                           |    
//            | PCMapper |--------->| SpikeGen.| (contains a memory)                       |    
//            |          |----------|          |                                           |    
//            +----------+ SG_conf, +----------+                                           |    
//                | |      SG_program_mem |                                                |    
//                | |                     |                                                |    
//                | | TM_conf             | time_unit_pulse                                |    
//                | |   +-----------+     |                                                |    
//                | |   |           |-----+                                                |   
//                | +---|  TimeMgr  |     |                                                |
//                |     |           |-----|------------------------------------------------+    
//                |     +-----------+     |                                          stall_dn      
//                |          |            |                                                   
//                |          |            |                                                   
//        SF_conf |          |      +----------+                                              
//                |          |      |          |                                              
//                +----------|------|SpikeFilt.| (contains a memory)                          
//                           | +----|          |                                              
//                           | |    +----------+                                              
//         send_HB_pulse_up, | |          ^                                                   
//             time_elapsed  | |          | SF_tags_in 
//                           | |          |            
//            +----------+   | |          |            +-------------+ SF_BD_data_in          
//            |          |---+ |          +------------|TagCtUnpacker|<----|\                                   
// PC_out <---| PCPacker |<----+ PCPacker_SF_data_in   +-------------+     | |<--------------------- BD_in
//  40b       |          |<------------------------------------------------|/                         34b       
//            +----------+            PCPacker_BD_data_in                   (module BDInSplit)                  
//                                                                                            

/////////////////////////////////////////////
// PCMapper signals, FPGA config data

// between PCParser and mapper
logic [Nreg-1:0][Nconf-1:0] conf_regs;
logic [Nreg-1:0][Nconf-1:0] conf_reg_reset_vals;
ChannelArray #(Nconf, Nchan) conf_channels(); 

// PCMapper outputs, internal config data
// conf registers 
SpikeFilterConf #(N_SF_filts, N_SF_state) SF_conf();
SpikeGeneratorConf #(N_SG_gens) SG_conf();
TimeMgrConf #(N_TM_unit, N_TM_time) TM_conf();
// conf channels
SpikeGeneratorProgChannel #(N_SG_gens, N_SG_period, N_SG_tag) SG_program_mem();

// time-related signals
logic time_unit_pulse;
logic send_HB_up_pulse;
logic stall_dn;
logic [N_TM_time-1:0] time_elapsed;

// data channels: PC -> BD
Channel #(NBDdata_out) PCParser_BD_data_out();
TagCtChannel #(Ntag, Nct) SG_tags_out();
Channel #(NBDdata_out) SG_BD_data_out();
Channel #(NBDdata_out) PCParser_SG_merged();

// data channels: BD -> PC
Channel #(NBDdata_in) SF_BD_data_in();
TagCtChannel #(Ntag, Nct) SF_tags_in();
Channel #(NBDdata_in) PCPacker_BD_data_in();
SpikeFilterOutputChannel #(N_SF_filts, N_SF_state) PCPacker_SF_data_in();

/////////////////////////////////////////////
// Config/FPGA state modules

PCParser #(
  .NPCin(NPCin),
  .NBDdata(NBDdata_out),
  .Nconf(Nconf),
  .Nreg(Nreg),
  .Nchan(Nchan)) 
PC_parser(
  conf_regs,
  conf_channels,
  PCParser_BD_data_out,
  PC_in,
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

// SG TagCtPacker
TagCtPacker #(
  .Ntag(Ntag),
  .Nct(Nct),
  .NBDdata(NBDdata_out))
SG_tag_ct_packer(
  SG_BD_data_out,
  SG_tags_out);

// PCParser/SG merge
ChannelMerge PCParser_SG_merge(
  PCParser_SG_merged,
  SG_BD_data_out,
  PCParser_BD_data_out,
  clk, reset);

// BD_out staller
ChannelStaller BD_out_stall(
  BD_out,
  PCParser_SG_merged,
  stall_dn);

/////////////////////////////////////////////
// BD -> PC datapath

// BDInSplit
BDInSplit #(NBDdata_in) 
BD_in_split(SF_BD_data_in, PCPacker_BD_data_in, BD_in, clk, reset);

// SpikeFilter
SpikeFilterArray #(
  .Nfilts(N_SF_filts),
  .Nstate(N_SF_state),
  .Nct(N_SF_ct))
SF_array(
  PCPacker_SF_data_in,
  SF_tags_in,
  time_unit_pulse,
  SF_conf,
  clk, reset);

// PCPacker
PCPacker #(
  .NPCout(NPCout),
  .NBDdata(NBDdata_in),
  .Ntime(N_TM_time),
  .N_SF_filts(N_SF_filts),
  .N_SF_state(N_SF_state))
PC_packer(
  PC_out,
  PCPacker_BD_data_in,
  send_HB_up_pulse,
  time_elapsed,
  PCPacker_SF_data_in,
  clk, reset);

endmodule

///////////////////////////
// TESTBENCH

module Core_tb;

parameter NPCout = 40;
parameter NPCin = 24;

parameter NBDdata_in = 34;
parameter NBDdata_out = 22;


// PC-side
Channel #(NPCin) PC_in();
Channel #(NPCout) PC_out();

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

RandomChannelSrc #(.N(NPCin)) PC_in_src(PC_in, clk, reset);
ChannelSink PC_out_sink(PC_out, clk, reset);

RandomChannelSrc #(.N(NBDdata_in)) BD_in_src(BD_in, clk, reset);
//assign BD_in.v = 0;
//assign BD_in.d = 'X;

ChannelSink BD_out_sink(BD_out, clk, reset);

Core dut(.*);

endmodule
