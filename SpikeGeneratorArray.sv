`include "SpikeGenMem.v"

// Creates a variable number of uniform spike streams.
// Uses a blockram to store the counter states, cycles through the blockram
// performing updates. Must be able to cycle through all states within one time_unit. 
// Can perform one update per clock cycle
module SpikeGeneratorArray #(parameter Ngens = 8, parameter Nperiod = 32, parameter Ntag = 11, parameter Nct = 10) (
  PassiveChannel out, // Ntag + Nct wide

  input unit_pulse, // kicks off update of all generators

  input [Ngens-1:0] gens_used, // total number of generators used
  input [(2**Ngens)-1:0] gens_en, // enable signal for each generator

  Channel program_mem;

  input clk,
  input reset);

// count mem 
// stores one time unit count per generator
// stores one spike count per generator (increments if output is stalling)

NBperiod = 4; // bytes per period field
NBtag = 2; // bytes per tag/enable field
NBct = 2;     // bytes per count field
parameter NBmem = NBperiod * 2 + NBtag + NBct;
parameter Nmem = NBmem * 8;

logic [Ngens-1:0] mem_wr_addr;
logic [Nmem-1:0] mem_wr_data;
logic [NBmem-1:0] mem_wr_bytemask;
logic mem_wr_en;
logic [Ngens-1:0] mem_rd_addr;
logic [Nmem-1:0] mem_rd_data;
logic mem_rd_en;

SpikeGeneratorMem mem(
  .wraddress(mem_wr_addr),
  .data(mem_wr_data),
  // need bytemask
  .wren(mem_wr_en),
  .raddress(mem_rd_addr),
  .q(mem_rd_data),
  .rden(mem_rd_en),
  .clock(clk));

// can be programmed in the READY state
// can't interrupt UPDATE for risk of breaking timing
enum {READY, FIRST_UPDATE, UPDATE, LAST_UPDATE} state, next_state;
logic [Ngens-1:0] last_gen_idx, gen_idx, next_gen_idx;

always_ff @(posedge clk, posedge reset)
  if (reset == 1) begin
    state <= READY;
    last_gen_idx <= 'X;
    gen_idx <= 0;
  end
  else begin
    state <= next_state;
    last_gen_idx <= gen_idx;
    gen_idx <= next_gen_idx;
  end

// gen_idx_p1 is used in a couple places
logic [Ngen-1:0] gen_idx_p1;
assign gen_idx_p1 = gen_idx + 1;

// next_state computation
always_comb
  unique case (state)
    READY:
      if (unit_pulse == 1) begin
        next_state = UPDATE;
        next_gen_idx = 0;
      end
      else begin
        next_state = READY;
        next_gen_idx = 0;
      end
    FIRST_UPDATE: begin
      // read from gen_idx (= 0), no write
      assert (unit_pulse == 0); // failing means we aren't fast enough to update every generator in one time unit
      next_state = UPDATE;
      next_gen_idx = gen_idx_p1;
    end
    UPDATE: begin
      assert (unit_pulse == 0); // failing means we aren't fast enough to update every generator in one time unit
      // we are in this state for Ngen - 1 cycles
      // each cycle, we read from gen_idx and write to last_gen_idx
      if (gen_idx_p1 < gens_used) begin // note _p1
        next_state = UPDATE;
        next_gen_idx = gen_idx_p1;
      end
      else begin
        next_state = LAST_UPDATE;
        next_gen_idx = gen_idx_p1;
      end
    end
    LAST_UPDATE: begin
      // write to last_gen_idx
      assert (unit_pulse == 0); // failing means we aren't fast enough to update every generator in one time unit
      next_state = READY;
      next_gen_idx = 0;
    end
  endcase

// break apart program_mem
logic [Ngen-1:0] prog_addr;
logic [Nmem-1:0] prog_data;
assign {prog_data, prog_addr} = program_mem.d;

// break apart memory words
logic [8*NBperiod-1:0] curr_tick, next_tick, period;
logic [(8*NBtag-1)-1:0] tag;
logic [8*NBct-1:0] curr_ct, next_ct;
logic [Nmem-1:0] mem_writeback;

assign {curr_tick, period, tag, curr_ct} = mem_rd_data;
assign mem_writeback = {next_tick, (NMem-8*(NBperiod+NBtag+NBct)){1'bX}, next_ct};

logic [8*NBct-1:0] curr_ct_p1;
assign curr_ct_p1 = curr_ct + 1;

// writeback datapath and output
// if output is ready, and count > 0, then send current count
always_comb
  case (state)
    READY: begin
      next_tick = 'X;
      next_ct = 'X;
      out.v = 0;
      out.d = 'X;
    end
    DEFAULT: begin
      if (curr_tick < period) begin
        // ct won't increase
        next_tick = curr_tick + 1;
        // if there's leftover count, try to send it if we can
        if (curr_ct > 0 && out.r == 1) begin 
          out.v = 1;
          out.d = {tag, curr_ct};
          next_ct = 0;
        end
        // nothing to send
        else begin
          out.v = 0;
          out.d = 'X;
          next_ct = curr_ct;
        end
      end
      else begin
        // ct will increase
        next_tick = 1;
        // can send, try to send everything
        if (out.r == 1) begin 
          out.v = 1;
          out.d = {tag, curr_ct_p1};
          next_ct = 0;
        end
        // can't send, increment count
        else begin
          out.v = 0;
          out.d = 'X;
          next_ct = curr_ct_p1;
        end
      end
    end
  endcase

// capture mem_writeback in a register to use for next cycle's write
logic [Nmem-1:0] last_mem_writeback;
always_ff @(posedge clk, posedge reset)
  if (reset == 1)
    last_mem_writeback <= 0;
  else
    last_mem_write <= mem_writeback;

// memory inputs
always_comb
  unique case (state)
    READY: begin
      // if we're being given a programming input, write to memory
      if (program_mem.v == 1) begin
        mem_wr_en = 1;
        mem_wr_addr = prog_addr;
        mem_wr_data = prog_data;
        mem_wr_bytemask = '1;
      end
      // otherwise, just wait
      else begin
        mem_wr_en = 0;
        mem_wr_addr = 'X;
        mem_wr_data = 'X;
        mem_wr_bytemask = 'X;
      end
      // no reads in READY
      mem_rd_en = 0;
      mem_rd_addr = 'X;
    end
    FIRST_UPDATE: begin
      mem_rd_en = 1;
      mem_rd_addr = gen_idx;
    end
    UPDATE: begin
      mem_rd_en = 1;
      mem_rd_addr = gen_idx;

      mem_wr_en = 1;
      mem_wr_addr = last_gen_idx;
      mem_wr_bytemask = {NBperiod{1'b1}, (NBMem-NBperiod-NBct){1'b0}, NBct{1'b1}};
      mem_wr_data = last_mem_writeback;
    end
    LAST_UPDATE begin
      mem_wr_en = 1;
      mem_wr_addr = last_gen_idx;
      mem_wr_bytemask = {NBperiod{1'b1}, (NBMem-NBperiod-NBct){1'b0}, NBct{1'b1}};
      mem_wr_data = last_mem_writeback;
    end
  endcase


