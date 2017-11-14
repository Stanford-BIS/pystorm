`define SIMULATION
`include "../src/SpikeGeneratorArray.sv"
`include "ChannelSrcSink.sv"

module SpikeGeneratorArray_tb;

parameter Ngens = 8;
parameter Nperiod = 16;
parameter Ntag = 11;
parameter Nct = 10;
parameter Nmem = Nperiod * 2 + Ntag;

TagCtChannel out(); // Ntag + Nct wide

SpikeGeneratorConf #(Ngens) conf();

SpikeGeneratorProgChannel program_mem();

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

// unit_pulse
parameter ClksPerUnit = 32;
logic unit_pulse = 0;
initial begin
  forever begin
    #(ClksPerUnit*Tclk) @(posedge clk) unit_pulse <= 1;
    @(posedge clk) unit_pulse <= 0;
  end
end

// receiver
Channel #(Ntag + Nct) packed_out();
assign packed_out.v = out.v;
assign packed_out.d = {out.tag, out.ct};
assign out.a = packed_out.a;

ChannelSink #(.ClkDelaysMin(0), .ClkDelaysMax(10)) out_sink(packed_out, clk, reset);

// stimulus
initial begin
  @(posedge reset)
  conf.gens_used <= 0;
  conf.gens_en <= '0;

  // program gen 0 -> tag 512
  #(Tclk * 10) 
  @(posedge clk) 
  program_mem.v <= 1;
  program_mem.gen_idx <= 0;
  program_mem.period <= 2;
  program_mem.ticks <= 0;
  program_mem.tag <= 512;

  @(posedge program_mem.a);
  @(posedge clk) 
  program_mem.v <= 0;
  program_mem.gen_idx <= 'X;
  program_mem.period <= 'X;
  program_mem.ticks <= 'X;
  program_mem.tag <= 'X;

  // program gen 1 -> tag 513
  #(Tclk * 10) 
  @(posedge clk) 
  program_mem.v <= 1;
  program_mem.gen_idx <= 1;
  program_mem.period <= 4;
  program_mem.ticks <= 2;
  program_mem.tag <= 513;

  @(posedge program_mem.a);
  @(posedge clk) 
  program_mem.v <= 0;
  program_mem.gen_idx <= 'X;
  program_mem.period <= 'X;
  program_mem.ticks <= 'X;
  program_mem.tag <= 'X;

  // set conf.gens_used/conf.gens_en
  @(posedge clk)
  conf.gens_used <= 2;
  conf.gens_en <= 3;

  // test enable by disabling the first gen
  #(Tclk * 300)
  @(posedge clk)
  conf.gens_en <= 2;

end

SpikeGeneratorArray dut(.*);

endmodule

