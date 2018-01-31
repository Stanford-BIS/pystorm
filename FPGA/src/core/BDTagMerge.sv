`include "../lib/Channel.svh"
`include "../lib/ChannelUtil.svh"
`include "../lib/Interfaces.svh"

module BDTagMerge #(
  parameter NBDData_in = 20,
  parameter Ncode = 6, // leaf code
  parameter Ntag = 11,
  parameter Nct = 9) (

  UnencodedBDWordChannel BD_out,
  UnencodedBDWordChannel BD_in,
  TagCtChannel tag_ct_in,
  input clk, reset);

Channel #(NBDData_in + Ncode) BD_out_packed();
Channel #(NBDData_in + Ncode) BD_in_packed();
Channel #(NBDData_in + Ncode) tag_ct_in_packed(); // adds the code

localparam logic [Ncode-1:0] tag_ct_code = 30;

assign tag_ct_in_packed.v = tag_ct_in.v;
assign tag_ct_in_packed.d = {tag_ct_in.tag, tag_ct_in.ct, tag_ct_code};
assign tag_ct_in.a = tag_ct_in_packed.a;

assign BD_in_packed.v = BD_in.v;
assign BD_in_packed.d = {BD_in.payload, BD_in.leaf_code};
assign BD_in.a = BD_in_packed.a;

ChannelMerge base(BD_out_packed, BD_in_packed, tag_ct_in_packed, clk, reset);

assign BD_out.v = BD_out_packed.v;
assign {BD_out.payload, BD_out.leaf_code} = BD_out_packed.d;
assign BD_out_packed.a = BD_out.a;

endmodule
