`include "Channel.svh"

// Encapsulated BD handshakers and clock-domain-crossing
// FIFO for upstream and downstream.
//
// BD handshakers (and BD) use BD_clk, which is slower
// than core_clk and may also be skewed to account for 
// the output pad delays. Generated by PLL.

module BDIfc (
  // Core side
  Channel core_out, // output from us
  Channel core_in,  // input to us
  
  // BD side
  output logic        BD_out_clk,
  input               BD_out_ready,
  output logic        BD_out_valid,
  output logic [20:0] BD_out_data,

  output logic        BD_in_clk,
  output logic        BD_in_ready,
  input               BD_in_valid,
  input [33:0]        BD_in_data,

  input core_clk, BD_out_clk, BD_in_clk, reset
  );

localparam NBDin = 21;
localparam NBDout = 34;
// note: in/out is inconsistent here
// NBDin/out refers to BD's input/output
// not the inputs/outputs to THIS MODULE
// which is what everything else is based on

// signals between FIFO and BD ifc
Channel #(NBDin) core_out_slow();
Channel #(NBDout) core_in_slow();

/////////////////////////////////////
// downstream FIFO

logic [NBDin-1:0] out_FIFO_data_in;
logic             out_FIFO_wr_en;
logic             out_FIFO_wr_full;
logic             out_FIFO_wr_clk;

logic [NBDin-1:0] out_FIFO_data_out;
logic             out_FIFO_rd_ack;
logic             out_FIFO_rd_empty;
logic             out_FIFO_rd_clk;

BDOutFIFO BD_out_FIFO(
  .data(out_FIFO_data_in),
  .rdclk(out_FIFO_rd_clk),
  .rdreq(out_FIFO_rd_ack),
  .wrclk(out_FIFO_wr_clk),
  .wrreq(out_FIFO_wr_en),
  .q(out_FIFO_data_out),
  .rdempty(out_FIFO_rd_empty),
  .wrfull(out_FIFO_wr_full));

// handshake FIFO input
assign out_FIFO_wr_clk  = core_clk;
assign out_FIFO_data_in = core_in.d;
assign out_FIFO_wr_en   = core_in.v & ~out_FIFO_wr_full;
assign core_in.a        = core_in.v & ~out_FIFO_wr_full;

// handshake FIFO output
assign out_FIFO_rd_clk  = BD_clk;
assign core_in_slow.d   = out_FIFO_data_out;
assign out_FIFO_rd_ack  = core_in_slow.a;
assign core_in_slow.v   = ~out_FIFO_rd_empty;

/////////////////////////////////////
// upstream FIFO

logic [NBDout-1:0] in_FIFO_data_in;
logic              in_FIFO_wr_en;
logic              in_FIFO_wr_full;
logic              in_FIFO_wr_clk;

logic [NBDout-1:0] in_FIFO_data_out;
logic              in_FIFO_rd_ack;
logic              in_FIFO_rd_empty;
logic              in_FIFO_rd_clk;

BDInFIFO BD_in_FIFO(
  .data(out_FIFO_data_in),
  .rdclk(out_FIFO_rd_clk),
  .rdreq(out_FIFO_rd_ack),
  .wrclk(out_FIFO_wr_clk),
  .wrreq(out_FIFO_wr_en),
  .q(out_FIFO_data_out),
  .rdempty(out_FIFO_rd_empty),
  .wrfull(out_FIFO_wr_full));

// handshake FIFO input
assign in_FIFO_wr_clk  = BD_clk;
assign in_FIFO_data_in = core_in_slow.d;
assign in_FIFO_wr_en   = core_in_slow.v & ~in_FIFO_wr_full;
assign core_in_slow.a  = core_in_slow.v & ~in_FIFO_wr_full;

// handshake FIFO output
assign in_FIFO_rd_clk  = core_clk;
assign core_in.d       = in_FIFO_data_out;
assign in_FIFO_rd_ack  = core_in.a;
assign core_in.v       = ~in_FIFO_rd_empty;

/////////////////////////////////////
// downstream handshaker

FPGA_TO_BD bd_down_ifc(
  .bd_channel(core_out_slow),
  .valid(BD_out_valid),
  .data(BD_out_data),
  .ready(BD_out_ready),
  .reset(user_reset),
  .clk(BD_clk));

/////////////////////////////////////
// upstream handshaker

BD_TO_FPGA bd_up_ifc(
  .bd_channel(core_in_slow),
  .valid(BD_in_valid),
  .data(BD_in_data),
  .ready(BD_in_ready),
  .reset(user_reset),
  .clk(BD_clk));

endmodule
