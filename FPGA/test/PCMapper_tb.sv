`define SIMULATION
`include "../src/core/PCParser.sv"
`include "../src/core/PCMapper.sv"
`include "ChannelSrcSink.sv"

// testbench includes PCParser

module PCMapper_tb;

///////////////////////
// PCParser + shared pars

parameter NPCin = 24;
parameter NBDdata = 21;
parameter Nconf = 16;
parameter Nreg = 32;
parameter Nchan = 2;

///////////////////////
// PCMapper pars

// parameters for SpikeFilterArray config
parameter N_SF_filts = 10;
parameter N_SF_state = 27;
parameter N_SF_ct = 9;

// parameters for SpikeGeneratorArray config
parameter N_SG_gens = 8;
parameter N_SG_period = 16;
parameter N_SG_tag = 11;

// paramters for TimeMgr
parameter N_TM_time_hi = 20;
parameter N_TM_time_lo = 20;
parameter N_TM_unit = 16;

// reset values for PCParser
logic [Nreg-1:0][Nconf-1:0] conf_reg_reset_vals;

// SpikeFilter
SpikeFilterConf SF_conf();

// SpikeGenerator
ProgramSpikeGeneratorChannel SG_program_mem();
SpikeGeneratorConf SG_conf();

// TimeMgr
TimeMgrConf TM_conf();

// TagSplit
TagSplitConf TS_conf();

// BD IO
BDIOConf BD_conf();

// inputs from PCParser
logic [Nreg-1:0][Nconf-1:0] conf_reg_out;
ChannelArray #(Nconf, Nchan) conf_channel_out(); // Nchan channels; Nconf wide
  
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

// PCParser input
Channel #(NPCin) PC_in();

// PCParser passthrough output to BD
UnencodedBDWordChannel BD_data_out();

// PC sender
RandomChannelSrc #(.N(NPCin)) PC_src(PC_in, clk, reset);

// BD receiver
Channel #(26) BD_data_out_packed();
assign BD_data_out_packed.d = {BD_data_out.leaf_code, BD_data_out.payload};
assign BD_data_out_packed.v = BD_data_out.v;
assign BD_data_out.a = BD_data_out_packed.a;
ChannelSink BD_sink(BD_data_out_packed, clk, reset);

// SG_program_mem sink
parameter N_SG_program_mem = N_SG_gens + 2 * N_SG_period + N_SG_tag;
Channel #(N_SG_program_mem) SG_program_mem_flat();
assign SG_program_mem_flat.d = {SG_program_mem.gen_idx, SG_program_mem.period, SG_program_mem.ticks, SG_program_mem.tag};
assign SG_program_mem_flat.v = SG_program_mem.v;
assign SG_program_mem.a = SG_program_mem_flat.a;
ChannelSink SG_program_mem_sink(SG_program_mem_flat, clk, reset);

PCParser #(NPCin, NBDdata, Nconf, Nreg, Nchan) parser(.*);

PCMapper #(
  Nconf,
  Nreg,
  Nchan,
  N_SF_filts,
  N_SF_state,
  N_SF_ct,
  N_SG_gens,
  N_SG_period,
  N_SG_tag,
  N_TM_time_hi,
  N_TM_time_lo,
  N_TM_unit) dut(.*);

endmodule

