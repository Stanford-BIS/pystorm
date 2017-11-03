`define SIMULATION
`include "../src/core/BDTagSplit.sv"
`include "ChannelSrcSink.sv"

module BDTagSplit_tb;

parameter NBDdata_in = 34;
parameter NBDpayload = 32;
parameter NBDcode = 4;
parameter Ntag = 11;
parameter Nct = 9;

TagCtChannel tag_out();
DecodedBDWordChannel other_out();
DecodedBDWordChannel BD_in();
TagSplitConf conf();

Channel #(Ntag + Nct) tag_out_flat();
Channel #(NBDpayload + NBDcode) other_out_flat();
Channel #(NBDpayload + NBDcode) BD_in_flat();

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

// conf.report_tags
parameter Treport_tags = 100;
always #(Treport_tags/2) conf.report_tags = ~conf.report_tags;
initial conf.report_tags = 0;

assign tag_out_flat.d = {tag_out.ct, tag_out.tag};
assign tag_out_flat.v = tag_out.v;
assign tag_out.a = tag_out_flat.a;

assign other_out_flat.d = {other_out.leaf_code, other_out.payload};
assign other_out_flat.v = other_out.v;
assign other_out.a = other_out_flat.a;

assign {BD_in.leaf_code, BD_in.payload} = BD_in_flat.d;
assign BD_in.v = BD_in_flat.v;
assign BD_in_flat.a = BD_in.a;

RandomChannelSrc #(.N(NBDpayload + NBDcode)) BD_in_src(BD_in_flat, clk, reset);
ChannelSink tag_out_sink(tag_out_flat, clk, reset);
ChannelSink other_out_sink(other_out_flat, clk, reset);

BDTagSplit dut(.*);

endmodule

