`include "Channel.svh"
`include "Interfaces.svh"

module TagCtPacker #(parameter Ntag = 11, parameter Nct = 9, parameter NBDdata = 22) (
  Channel BD_data_out,
  TagCtChannel tag_ct_data_in);

assign BD_data_out.v = tag_ct_data_in.v;
assign BD_data_out.d = {tag_ct_data_in.tag, tag_ct_data_in.ct, 1'b0}; // route "1" means "RI(input from 'router'):FIFO tag input"
assign tag_ct_data_in.a = BD_data_out.a;

endmodule
  
