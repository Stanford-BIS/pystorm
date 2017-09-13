`include "../src/PCParser.sv"
`include "ChannelSrcSink.sv"

module PCParser_tb;

parameter NPCin = 32;
parameter NBDbiggest_data = 20;
parameter Nconf = 16;
parameter Nreg = 4;
parameter Nchan = Nreg;

// output registers
logic [Nreg-1:0][Nconf-1:0] conf_reg_out;

// output channels
ChannelArray #(Nconf, Nchan) conf_channel_out(); 
Channel #(Nconf) conf_channel_out_unpacked[Nchan-1:0](); 
UnpackChannelArray #(Nchan) conf_unpacker(conf_channel_out, conf_channel_out_unpacked);

// passthrough output to BD
UnencodedBDWordChannel BD_data_out();

// input; from PC
Channel #(NPCin) PC_in();

logic [Nreg-1:0][Nconf-1:0] conf_reg_reset_vals = '0;

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

// PC sender
RandomChannelSrc #(.N(NPCin)) PC_src(PC_in, clk, reset);

// BD receiver
Channel #(26) BD_data_out_packed();
assign BD_data_out_packed.d = {BD_data_out.leaf_code, BD_data_out.payload};
assign BD_data_out_packed.v = BD_data_out.v;
assign BD_data_out.a = BD_data_out_packed.a;
ChannelSink BD_sink(BD_data_out_packed, clk, reset);

// conf_channel receivers
ChannelSink chan_sinks[Nchan-1:0](conf_channel_out_unpacked, clk, reset);

PCParser #(NPCin, NBDbiggest_data, Nconf, Nreg, Nchan) dut(.*);

endmodule

