`include "../lib/Interfaces.svh"
`include "../lib/Channel.svh"
`include "../lib/ChannelUtil.svh"

module BDTagSplit #(
  parameter NBDdata_in = 34,
  parameter Nglobal = 12,
  parameter Ntag = 11,
  parameter Nct = 9) (

  TagCtChannel tag_out,
  GlobalTagCtChannel global_tag_out,
  DecodedBDWordChannel other_out,
  DecodedBDWordChannel BD_in,
  TagSplitConf conf,
  input clk, reset);

localparam unsigned RO_ACC_code = 11;
localparam unsigned RO_TAT_code = 12;
localparam unsigned GO_HOME_gt = 0; // ADD GO HOME ROUTE

//get global tag kinda cleanly
logic [Nglobal - 1:0] global_tag;
assign global_tag = BD_in.payload[Nglobal + Ntag + Nct - 1 : Ntag + Nct];

//this guy is just a spaceholder for the combinational logic
logic [Nglobal - 1:0] global_tag_spaceholder;

//create booleans
//check leaf code for tagness
logic is_tag;
assign is_tag = (BD_in.leaf_code == RO_ACC_code) | (BD_in.leaf_code == RO_TAT_code);

//check global tag value for go home (also check valid)
logic send_to_global_tag;
assign send_to_global_tag = BD_in.v & is_tag & (global_tag != GO_HOME_gt);

//use settings to determine other_out behavior
logic send_to_other_out;
assign send_to_other_out = BD_in.v & (is_tag & (conf.report_tags == 1) | ~is_tag);

//detemine tag sending
logic send_to_tag_out;
assign send_to_tag_out = BD_in.v & is_tag;

//send global tag to global tag out iff global tag != home
always_comb
  if(send_to_global_tag == 1) begin
    global_tag_out.v = 1;
    {global_tag_out.global_tag, global_tag_out.tag, global_tag_out.ct} = BD_in.payload;
  end
  else begin
    global_tag_out.v = 0;
    {global_tag_out.global_tag, global_tag_out.tag, global_tag_out.ct} = 'X;
  end

//otherwise, send to the old stuff
always_comb
  if (send_to_tag_out == 1 & send_to_global_tag == 0) begin
    tag_out.v = 1;
    {global_tag_spaceholder, tag_out.tag, tag_out.ct} = BD_in.payload;
  end
  else begin
    tag_out.v = 0;
    {global_tag_spaceholder, tag_out.tag, tag_out.ct} = 'X;
  end

always_comb
  if (send_to_other_out == 1 & send_to_global_tag == 0) begin
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
  if (send_to_global_tag == 1)
    BD_in.a = global_tag_out.a;
  else if (send_to_tag_out == 1 && send_to_other_out == 1)
    BD_in.a = tag_out.a & other_out.a;
  else if (send_to_tag_out == 1 && send_to_other_out == 0)
    BD_in.a = tag_out.a;
  else if (send_to_tag_out == 0 && send_to_other_out == 1)
    BD_in.a = other_out.a;
  else
    BD_in.a = 0;

endmodule
