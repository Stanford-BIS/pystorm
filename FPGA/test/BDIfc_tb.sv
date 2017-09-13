`include "../src/BDIfc.sv"
`include "ChannelSrcSink.sv"

module BDIfc_tb;

// BD ifc
wire        BD_out_ready;
wire        BD_out_valid;
wire [20:0] BD_out_data;

wire        BD_in_ready;
wire        BD_in_valid;
wire[33:0]  BD_in_data;

// clocks
logic core_clk;
parameter Tcore_clk = 10;
always #(Tcore_clk/2) core_clk = ~core_clk;
initial core_clk = 0;

logic BD_out_clk_int;
parameter TBD_out_clk_int = 100;
always #(TBD_out_clk_int/2) BD_out_clk_int = ~BD_out_clk_int;
initial BD_out_clk_int = 0;

logic BD_in_clk_int;
assign BD_in_clk_int = BD_out_clk_int;

// reset
logic reset;
initial begin
  reset <= 0;
  #(10) reset <= 1;
  #(100) reset <= 0;
end

// channels to core
localparam NBDdn = 21;
localparam NBDup = 34;
// note: in/out is inconsistent here
// NBDdn/out refers to BD's input/output
// not the inputs/outputs to THIS MODULE
// which is what everything else is based on

Channel #(NBDup) core_up();
Channel #(NBDdn) core_dn();

RandomChannelSrc #(.N(NBDup)) core_dn_src(core_dn, core_clk, reset);
ChannelSink core_up_sink(core_up, core_clk, reset);

// BD src/sink
BD_Source #(.NUM_BITS(34)) src(BD_in_data, BD_in_valid, BD_in_ready, reset, BD_in_clk_int);
BD_Sink #(.NUM_BITS(21)) sink(BD_out_ready, BD_out_valid, BD_out_data, reset, BD_out_clk_int);

// DUT
BDIfc dut(.*);

endmodule
