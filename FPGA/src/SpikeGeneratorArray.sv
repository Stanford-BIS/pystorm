`include "Interfaces.svh"
`include "Channel.svh"
// for quartus, we add external IP to the project
`ifdef SIMULATION
  `include "../quartus/SpikeGeneratorMem.v"
`endif

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

localparam Nmem = Nperiod * 2 + Ntag;
localparam Nmem_bytes = 6;
localparam Nmem_full = Nmem_bytes * 8;

// stores one time unit count ("tick") per generator

logic [Ngens-1:0] mem_wr_addr;
logic [Nmem_full-1:0] mem_wr_data;
logic [Nmem_bytes-1:0] mem_wr_bytemask;
logic mem_wr_en;
logic [Ngens-1:0] mem_rd_addr;
logic [Nmem_full-1:0] mem_rd_data;
logic mem_rd_en;

SpikeGeneratorMem mem(
  .wraddress(mem_wr_addr),
  .data(mem_wr_data),
  .wren(mem_wr_en),
  .byteena_a(mem_wr_bytemask),
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
      assert (unit_pulse == 0); // failing means we aren't fast enough to update every generator in one time unit
      next_state <= UPDATE_C;
      next_gen_idx <= gen_idx;
    end

    UPDATE_C: begin
      // read data available from output register, do writeback, maybe send data
      assert (unit_pulse == 0); 
      if (stall_out) begin
        next_state <= UPDATE_C;
        next_gen_idx <= gen_idx;
      end
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
assign {tag, period, rd_tick} = mem_rd_data;

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
assign mem_writeback = {tag, period, wr_tick};

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
        mem_wr_data = {program_mem.tag, program_mem.period, program_mem.ticks};
        mem_wr_bytemask = 6'b111100; // don't actually overwrite the ticks

        mem_rd_en = 0;
        mem_rd_addr = 'X;
      end
      // otherwise, just wait
      else begin
        mem_wr_en = 0;
        mem_wr_addr = 'X;
        mem_wr_data = 'X;
        mem_wr_bytemask = 'X;

        mem_rd_en = 0;
        mem_rd_addr = 'X;
      end
    end

    UPDATE_A: begin
      // read
      mem_wr_en = 0;
      mem_wr_addr = 'X;
      mem_wr_data = 'X;
      mem_wr_bytemask = 'X;

      mem_rd_en = 1;
      mem_rd_addr = gen_idx;

    end

    UPDATE_B: begin
      // memory idle, waiting for read to come out and register
      mem_wr_en = 0;
      mem_wr_addr = 'X;
      mem_wr_data = 'X;
      mem_wr_bytemask = 'X;

      mem_rd_en = 0;
      mem_rd_addr = 'X;
    end

    UPDATE_C: begin
      // write back
      mem_wr_en = 1;
      mem_wr_addr = gen_idx;
      mem_wr_data = mem_writeback;
      mem_wr_bytemask = '1;

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


