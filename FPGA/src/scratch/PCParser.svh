`ifndef PCPARSER_SVH
`define PCPARSER_SVH

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

`endif
