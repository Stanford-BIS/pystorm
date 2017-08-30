`include "Channel.svh"
`include "Interfaces.svh"

module PCMapper #(
  // parameters of PCParser output
  parameter Nconf = 16,
  parameter Nreg = 32,
  parameter Nchan = 1,

  // parameters for SpikeFilterArray config
  parameter N_SF_filts = 10,
  parameter N_SF_state = 27,
  parameter N_SF_ct = 9,

  // parameters for SpikeGeneratorArray config
  parameter N_SG_gens = 8,
  parameter N_SG_period = 16,
  parameter N_SG_tag = 11,

  // paramters for TimeMgr
  parameter N_TM_time = 48,
  parameter N_TM_unit = 16) (

  // reset values for PCParser
  output logic [Nreg-1:0][Nconf-1:0] conf_reg_reset_vals,

  // SpikeFilter
  SpikeFilterConf SF_conf,

  // SpikeGenerator
  SpikeGeneratorProgChannel SG_program_mem,
  SpikeGeneratorConf SG_conf,

  // TimeMgr
  TimeMgrConf TM_conf,

  // inputs from PCParser
  input [Nreg-1:0][Nconf-1:0] conf_reg_out,
  ChannelArray conf_channel_out, // Nchan channels, Nconf wide
  
  input clk, reset);

/////////////////////////////////////////////////////
// register mapping, register resets

// compute register index offsets automatically

parameter N_SG_gens_en = 2**N_SG_gens;

parameter N_SF_filts_chunks   = N_SF_filts   % Nconf == 0 ? N_SF_filts   / Nconf : N_SF_filts   / Nconf + 1;
parameter N_SF_state_chunks   = N_SF_state   % Nconf == 0 ? N_SF_state   / Nconf : N_SF_state   / Nconf + 1;
parameter N_SF_ct_chunks      = N_SF_ct      % Nconf == 0 ? N_SF_ct      / Nconf : N_SF_ct      / Nconf + 1;
parameter N_SG_gens_chunks    = N_SG_gens    % Nconf == 0 ? N_SG_gens    / Nconf : N_SG_gens    / Nconf + 1;
parameter N_SG_gens_en_chunks = N_SG_gens_en % Nconf == 0 ? N_SG_gens_en / Nconf : N_SG_gens_en / Nconf + 1;
parameter N_SG_period_chunks  = N_SG_period  % Nconf == 0 ? N_SG_period  / Nconf : N_SG_period  / Nconf + 1;
parameter N_SG_tag_chunks     = N_SG_tag     % Nconf == 0 ? N_SG_tag     / Nconf : N_SG_tag     / Nconf + 1;
parameter N_TM_time_chunks    = N_TM_time    % Nconf == 0 ? N_TM_time    / Nconf : N_TM_time    / Nconf + 1;
parameter N_TM_unit_chunks    = N_TM_unit    % Nconf == 0 ? N_TM_unit    / Nconf : N_TM_unit    / Nconf + 1;

parameter SF_base                   = 0;
parameter SF_filts_used_idx         = SF_base + 0;
parameter SF_increment_constant_idx = SF_base + N_SF_filts_chunks;
parameter SF_decay_constant_idx     = SF_base + N_SF_filts_chunks + N_SF_state_chunks;

parameter SG_base                   = SF_base + N_SF_filts_chunks + 2*N_SF_state_chunks; // = 5
parameter SG_gens_used_idx          = SG_base + 0;
parameter SG_gens_en_idx            = SG_base + N_SG_gens_chunks;

parameter TM_base                   = SG_base + N_SG_gens_chunks + N_SG_gens_en_chunks; // = 23
parameter TM_unit_len_idx           = TM_base + 0; 
parameter TM_PC_time_elapsed_idx    = TM_base + N_TM_unit_chunks;
parameter TM_PC_send_HB_up_idx      = TM_base + N_TM_unit_chunks + N_TM_time_chunks;
parameter TM_PC_reset_time_idx      = TM_base + N_TM_unit_chunks + 2*N_TM_time_chunks;

// assign registers

assign SF_conf.filts_used         = conf_reg_out[SF_filts_used_idx        +:N_SF_filts_chunks];
assign SF_conf.increment_constant = conf_reg_out[SF_increment_constant_idx+:N_SF_state_chunks];
assign SF_conf.decay_constant     = conf_reg_out[SF_decay_constant_idx    +:N_SF_state_chunks];

assign SG_conf.gens_used          = conf_reg_out[SG_gens_used_idx         +:N_SG_gens_chunks];
assign SG_conf.gens_en            = conf_reg_out[SG_gens_en_idx           +:N_SG_gens_en_chunks];

assign TM_conf.unit_len           = conf_reg_out[TM_unit_len_idx          +:N_TM_unit_chunks];
assign TM_conf.PC_time_elapsed    = conf_reg_out[TM_PC_time_elapsed_idx   +:N_TM_time_chunks];
assign TM_conf.send_HB_up_every   = conf_reg_out[TM_PC_send_HB_up_idx     +:N_TM_time_chunks];
assign TM_conf.reset_time         = conf_reg_out[TM_PC_reset_time_idx];

// assign resets

assign conf_reg_reset_vals[SF_filts_used_idx        +:N_SF_filts_chunks]   = 0; // no filters enabled, in "decay mode"
assign conf_reg_reset_vals[SF_increment_constant_idx+:N_SF_state_chunks]   = 1;
assign conf_reg_reset_vals[SF_decay_constant_idx    +:N_SF_state_chunks]   = 0;

assign conf_reg_reset_vals[SG_gens_used_idx         +:N_SG_gens_chunks]    = 0; // all generators disabled
assign conf_reg_reset_vals[SG_gens_en_idx           +:N_SG_gens_en_chunks] = 0;

assign conf_reg_reset_vals[TM_unit_len_idx          +:N_TM_unit_chunks]    = 5000; // for 200 MHz clk, 10 us time resolution
assign conf_reg_reset_vals[TM_PC_time_elapsed_idx   +:N_TM_time_chunks]    = 0;
assign conf_reg_reset_vals[TM_PC_send_HB_up_idx     +:N_TM_time_chunks]    = 10; // send HB every 10 time units
assign conf_reg_reset_vals[TM_PC_reset_time_idx]                           = 0; 

assign conf_reg_reset_vals[Nreg-1:TM_PC_reset_time_idx+1] = 0;

/////////////////////////////////////////////////////
// Channel mapping, deserialization

// unpack ChannelArray
Channel conf_channel_out_unpacked[Nchan-1:0]();
UnpackChannelArray #(Nchan) conf_channel_unpacker(conf_channel_out, conf_channel_out_unpacked);

parameter Nchans_used = 1;

// pack channel so we can use deserializer
parameter N_SG_program_mem = N_SG_gens + 2 * N_SG_period + N_SG_tag;
Channel #(N_SG_program_mem) SG_program_mem_flat();
assign {SG_program_mem.gen_idx, SG_program_mem.period, SG_program_mem.ticks, SG_program_mem.tag} = SG_program_mem_flat.d;
assign SG_program_mem.v = SG_program_mem_flat.v;
assign SG_program_mem_flat.a = SG_program_mem.a;

Deserializer #(.Nin(Nconf), .Nout(N_SG_program_mem)) SG_program_mem_des(conf_channel_out_unpacked[0], SG_program_mem_flat, clk, reset);

// loop .v back onto .a for unused channels, making them handshake
generate
for (genvar i = Nchans_used; i < Nchan; i++)
  assign conf_channel_out_unpacked[i].a = conf_channel_out_unpacked[i].v;
endgenerate

endmodule


/////////////////////////////////////////////////////
// TESTBENCH (with PCParser)

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
Channel #(NBDdata) BD_data_out();

// PC sender
RandomChannelSrc #(.N(NPCin)) PC_src(PC_in, clk, reset);

// BD receiver
ChannelSink BD_sink(BD_data_out, clk, reset);

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

