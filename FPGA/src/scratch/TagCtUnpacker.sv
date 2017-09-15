`include "Channel.svh"
`include "Interfaces.svh"

module TagCtUnpacker #(parameter Ntag = 11, parameter Nct = 9, parameter NBDdata = 34) (
  TagCtChannel tag_ct_data_out,
  Channel BD_data_in);

logic [1:0] funnel_route; // discarded
logic [NBDdata - Ntag - Nct - 1:0] global_tag; // discarded

assign tag_ct_data_out.v = BD_data_out.v;
assign {funnel_route, global_tag, tag_ct_data_out.tag, tag_ct_data_out.ct} = BD_data_out.d;
assign BD_data_in.a = tag_ct_data_out.a;

endmodule
  
