`include "Interfaces.svh"
`include "Channel.svh"
`include "ChannelUtil.svh"

module BDTagSplit #(
  parameter NBDdata_in = 34,
  parameter Ntag = 11,
  parameter Nct = 9) (

  TagCtChannel tag_out,
  DecodedBDWordChannel other_out,
  DecodedBDWordChannel BD_in,
  TagSplitConf conf,
  input clk, reset);

localparam unsigned RO_ACC_code = 11;
localparam unsigned RO_TAT_code = 12;

logic [NBDdata_in - Ntag - Nct - 1:0] global_tag; // discarded

logic is_tag;
assign is_tag = (BD_in.leaf_code == RO_ACC_code) | (BD_in.leaf_code == RO_TAT_code);

logic send_to_other_out;
assign send_to_other_out = BD_in.v & (is_tag & (conf.report_tags == 1) | ~is_tag);

logic send_to_tag_out;
assign send_to_tag_out = BD_in.v & is_tag;

always_comb
  if (send_to_tag_out == 1) begin
    tag_out.v = 1;
    {global_tag, tag_out.tag, tag_out.ct} = BD_in.payload;
  end
  else begin
    tag_out.v = 0;
    {global_tag, tag_out.tag, tag_out.ct} = 'X;
  end

always_comb
  if (send_to_other_out == 1) begin
    other_out.v = 1;
    other_out.payload = BD_in.payload;
    other_out.leaf_code = BD_in.leaf_code;
  end
  else begin
    other_out.v = 0;
    other_out.payload = 'X;
    other_out.leaf_code = 'X;
  end

// handshake input
always_comb
  if (send_to_tag_out == 1 && send_to_other_out == 1)
    BD_in.a = tag_out.a & other_out.a;
  else if (send_to_tag_out == 1 && send_to_other_out == 0)
    BD_in.a = tag_out.a;
  else if (send_to_tag_out == 0 && send_to_other_out == 1)
    BD_in.a = other_out.a;
  else
    BD_in.a = 0;

endmodule
