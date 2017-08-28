`include "SpikeGeneratorMem.v"
`include "Interfaces.svh"
`include "Channel.svh"

// Creates a variable number of uniform spike streams.
// Uses a blockram to store the counter states, cycles through the blockram
// performing updates. Must be able to cycle through all states within one time_unit. 
// Can perform one update every three clock cycles
// (this implementation doesn't pipeline reads and writes)
module SpikeGeneratorArray #(parameter Ngens = 8, parameter Nperiod = 16, parameter Ntag = 11, parameter Nct = 10) (
  TagCtChannel out, // Ntag + Nct wide

  input unit_pulse, // kicks off update of all generators

  SpikeGeneratorConf conf,
  SpikeGeneratorProgChannel program_mem,

  input clk,
  input reset);

parameter Nmem = Nperiod * 2 + Ntag;

// stores one time unit count ("tick") per generator

logic [Ngens-1:0] mem_wr_addr;
logic [Nmem-1:0] mem_wr_data;
logic mem_wr_en;
logic [Ngens-1:0] mem_rd_addr;
logic [Nmem-1:0] mem_rd_data;
logic mem_rd_en;

SpikeGeneratorMem mem(
  .wraddress(mem_wr_addr),
  .data(mem_wr_data),
  .wren(mem_wr_en),
  .rdaddress(mem_rd_addr),
  .q(mem_rd_data),
  .rden(mem_rd_en),
  .clock(clk));

//////////////////////////////////////////////////////
// state computation

// if programming is attempted during an UPDATE, will stall until it completes
enum {READY_OR_PROG, UPDATE_A, UPDATE_B, UPDATE_C} state, next_state;
logic [Ngens-1:0] gen_idx, next_gen_idx;

always_ff @(posedge clk, posedge reset)
  if (reset == 1) begin
    state <= READY_OR_PROG;
    gen_idx <= 0;
  end
  else begin
    state <= next_state;
    gen_idx <= next_gen_idx;
  end

// gen_idx_p1 is used in a couple places
// make sure we only get one adder
logic [Ngens-1:0] gen_idx_p1;
assign gen_idx_p1 = gen_idx + 1;

logic stall_out;
assign stall_out = out.v & ~out.a;

// next_state computation
// we take two clock cycles
// it's possible to do one update/cycle, but it requires
// a pipeline with stalls
always_comb
  unique case (state)

    READY_OR_PROG: begin
      next_gen_idx = 0;
      if (unit_pulse == 1)
        next_state = UPDATE_A;
      else
        next_state = READY_OR_PROG;
    end

    UPDATE_A: begin
      // present read address to memory
      assert (unit_pulse == 0); // failing means we aren't fast enough to update every generator in one time unit
      if (conf.gens_en[gen_idx] == 1) begin
        next_gen_idx = gen_idx;
        next_state = UPDATE_B;
      end
      else begin
        next_gen_idx = gen_idx_p1;
        next_state = UPDATE_A;
      end
    end

    UPDATE_B: begin
      // read data comes out, goes into register
      // could try to write back this cycle, but we're not trying to push the timing
      assert (unit_pulse == 0); 
      next_state <= UPDATE_C;
      next_gen_idx <= gen_idx;
    end

    UPDATE_C: begin
      // read data available from output register, do writeback, maybe send data
      assert (unit_pulse == 0); 
      if (stall_out) begin
        next_state <= UPDATE_C;
        next_gen_idx <= gen_idx;
      else
        if (gen_idx_p1 < conf.gens_used) begin // note _p1
          next_state <= UPDATE_A;
          next_gen_idx <= gen_idx_p1;
        end
        else begin
          next_state <= READY_OR_PROG;
          next_gen_idx <= 'X;
        end
      end
    end

  endcase

//////////////////////////////////////////////////////
// handshake input
always_comb
  if (program_mem.v && state == READY_OR_PROG)
    program_mem.a = 1;
  else
    program_mem.a = 0;

//////////////////////////////////////////////////////
// writeback datapath

// unpack read data
// <rd_tick> is the number of periods elapsed since overflow for this generator
// <wr_tick> is therefore either <tick> + 1 or 1
logic [Nperiod-1:0] rd_tick, wr_tick, period;
logic [Ntag-1:0] tag;
logic [Nmem-1:0] mem_writeback;

// the output of the memory is registered, 
// so we see this TWO cycles after we assert rd_en
// the SAME cycle that we are writing back
assign {rd_tick, period, tag} = mem_rd_data;

logic emit_output;
always_comb
  if (rd_tick < period) begin
    wr_tick = rd_tick + 1;
    emit_output = 0;
  end
  else begin
    wr_tick = 1;
    emit_output = 1;
  end

// pack up writeback
parameter Nunmasked = Nmem - (Nperiod + Ntag);
assign mem_writeback = {wr_tick, period, tag};

//////////////////////////////////////////////////////
// memory inputs

// memory inputs
always_comb
  unique case (state)

    READY_OR_PROG: begin
      // if we're being given a programming input, write to memory
      if (program_mem.v == 1) begin
        mem_wr_en = 1;
        mem_wr_addr = program_mem.gen_idx;
        mem_wr_data = {program_mem.ticks, program_mem.period, program_mem.tag};

        mem_rd_en = 0;
        mem_rd_addr = 'X;
      end
      // otherwise, just wait
      else begin
        mem_wr_en = 0;
        mem_wr_addr = 'X;
        mem_wr_data = 'X;

        mem_rd_en = 0;
        mem_rd_addr = 'X;
      end
    end

    UPDATE_A: begin
      // read
      mem_wr_en = 0;
      mem_wr_addr = 'X;
      mem_wr_data = 'X;

      mem_rd_en = 1;
      mem_rd_addr = gen_idx;

    end

    UPDATE_B: begin
      // memory idle, waiting for read to come out and register
      mem_wr_en = 0;
      mem_wr_addr = 'X;
      mem_wr_data = 'X;

      mem_rd_en = 0;
      mem_rd_addr = 'X;
    end

    UPDATE_C: begin
      // write back
      mem_wr_en = 1;
      mem_wr_addr = gen_idx;
      mem_wr_data = mem_writeback;

      mem_rd_en = 0;
      mem_rd_addr = 'X;
    end

  endcase


/////////////////////////////////////////////////////////
// output

always_comb
  if (state == UPDATE_C && emit_output == 1) begin
    out.v = 1;
    out.tag = tag;
    out.ct = 1;
  end
  else begin
    out.v = 0;
    out.tag = 'X;
    out.ct = 'X;
  end

endmodule

/////////////////////////////////////////////////////////
// TESTBENCH
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
logic unit_pulse;
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

