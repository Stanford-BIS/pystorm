`include "Channel.svh"

module BDInSplit #(parameter NBDdata_in = 34) (
  Channel tag_out,
  Channel other_out,
  Channel BD_in,
  input clk, reset);

// SF_PCPacker split, built of two 2-way splits and a merge

// look at the two MSBs
// if they are "00" or "01", it's a a tag
  
// intermediate channels
Channel #(NBDdata_in) c_tat();
Channel #(NBDdata_in) c_acc();
Channel #(NBDdata_in) c_intermediate();

parameter logic [NBDdata_in-1:0] Mask = 2**(NBDdata_in-1) + 2**(NBDdata_in-2);
parameter logic [NBDdata_in-1:0] TAT_code = 0;
parameter logic [NBDdata_in-1:0] ACC_code = 2**(NBDdata_in-2);

ChannelSplit #(.N(NBDdata_in), .Mask(Mask), .Code0(TAT_code))
 split0(c_tat, c_intermediate, BD_in);

ChannelSplit #(.N(NBDdata_in), .Mask(Mask), .Code0(ACC_code))
 split1(c_acc, other_out, c_intermediate);

ChannelMerge merge(tag_out, c_tat, c_acc, clk, reset);

endmodule

///////////////////////////////
// TESTBENCH

module BDInSplit_tb;

parameter NBDdata = 34;

Channel #(NBDdata) tag_out();
Channel #(NBDdata) other_out();
Channel #(NBDdata) BD_in();

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

RandomChannelSrc #(.N(NBDdata)) BD_in_src(BD_in, clk, reset);
ChannelSink tag_out_sink(tag_out, clk, reset);
ChannelSink other_out_sink(other_out, clk, reset);

BDInSplit dut(.*);

endmodule

