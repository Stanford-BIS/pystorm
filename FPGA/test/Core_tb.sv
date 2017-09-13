`include "../src/Core.sv"
`include "ChannelSrcSink.sv"

module Core_tb;

parameter NPCcode = 8;
parameter NPCdata = 24;

parameter NBDdata_in = 34;
parameter NBDdata_out = 21;


// PC-side
Channel #(NPCcode + NPCdata) PC_in();
Channel #(NPCcode + NPCdata) PC_out();

// BD-side
Channel #(NBDdata_out) BD_out();
Channel #(NBDdata_in) BD_in();

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

RandomChannelSrc #(.N(NPCcode + NPCdata), .ClkDelaysMin(0), .ClkDelaysMax(4)) PC_in_src(PC_in, clk, reset);
ChannelSink #(.ClkDelaysMin(0), .ClkDelaysMax(1)) PC_out_sink(PC_out, clk, reset);

RandomChannelSrc #(.N(NBDdata_in)) BD_in_src(BD_in, clk, reset);

ChannelSink BD_out_sink(BD_out, clk, reset);

Core dut(.*);

endmodule
