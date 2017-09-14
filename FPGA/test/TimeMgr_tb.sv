`define SIMULATION
`include "../src/TimeMgr.sv"
`include "ChannelSrcSink.sv"

module TimeMgr_tb;

parameter Nunit = 16;
parameter Ntime = 48;

logic unit_pulse;
logic send_HB_up_pulse;
logic [Ntime-1:0] time_elapsed;

logic stall_dn;

TimeMgrConf #(Nunit, Ntime) conf();

logic clk;
logic reset;

parameter Tclk = 10;

always #(Tclk/2) clk = ~clk;

parameter ClksPerUnit = 8; // 4 clks is one unit
parameter UnitsPerHB = 4;

// PC clock comes in at random times
DatalessChannel send_PC_time();
DatalessChannelSrc #(.ClkDelaysMin(ClksPerUnit/2), .ClkDelaysMax(ClksPerUnit/2*3)) random_delay_maker(send_PC_time, clk, reset);
assign send_PC_time.a = 1; // always ack, we just want the delay

always @(posedge send_PC_time.v)
  conf.PC_time_elapsed <= conf.PC_time_elapsed + 1;

initial begin
  clk = 0;
  conf.unit_len = ClksPerUnit; 
  conf.send_HB_up_every = UnitsPerHB;
  conf.PC_time_elapsed = 0;

  reset = 1;
  #(Tclk) reset = 0;
  #(Tclk) conf.reset_time = 1;
  #(Tclk) conf.reset_time = 0;
end

TimeMgr #(.Ntime(Ntime), .Nunit(Nunit)) dut(.*);

endmodule
