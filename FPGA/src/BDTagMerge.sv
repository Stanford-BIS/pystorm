`include "Channel.svh"
`include "Interfaces.svh"

module BDTagMerge #(
  parameter NBDData_in = 20,
  parameter Ncode = 6,
  parameter Ntag = 11,
  parameter Nct = 9) (

  UnencodedBDWordChannel BD_out,
  UnencodedBDWordChannel BD_in,
  TagCtChannel tag_ct_in);

Channel #(NBDData_in + Ncode) BD_out_packed();
Channel #(NBDData_in + Ncode) BD_in_packed();
Channel #(Ntag + Nct) tag_ct_in_packed();

parameter [Ncode-1:0] logic tag_ct_code = 30;

assign tag_ct_in_packed.v = tag_ct_in.v;
assign tag_ct_in_packed.d = {tag_ct_in.tag, tag_ct_in.ct, tag_ct_code};
assign tag_ct_in.a = tag_ct_in_packed.a;

assign BD_in_packed.v = BD_in.v;
assign BD_in_packed.d = {BD_in.payload, BD_in.leaf_code};
assign BD_in.a = BD_in_packed.a;

ChannelMerge base(BD_out_packed, BD_in_packed, tag_ct_in_packed);

assign BD_out.v = BD_out_packed.v;
assign {BD_out.payload, BD_out.leaf_code} = BD_out_packed.d;
assign BD_out_packed.a = BD_out.a;

endmodule
