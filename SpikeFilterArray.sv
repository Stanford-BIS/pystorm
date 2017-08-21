`include "SpikeFilterMem.sv"
`include "Interfaces.svh"

// low-pass filters a variable number of spike streams
// uses a decaying exponential filter
module SpikeFilterArray #(parameter Nfilts = 10, parameter Nstate = 27, parameter Nct = 10) ( // 27 is the width of the cyclone 5's high-precision DSP block
  FilterOutputChannel out, 

  TagCtChannel in, 

  input update_pulse, // prompts all filters to update their values
  input filts_used,

  input [Nstate-1:0] increment_constant, // increment by this each time unit
  input [Nstate-1:0] decay_constant, // multiply by this each time unit

  input clk,
  input reset)

// filter is implementing tau * dx/dt = -x
// x(t) = exp(-t/tau) * x(0)
// rescale for unit area:
// x(t) = 1/tau * exp(-t/tau) * x(0)
//
// every time we get a spike, increment state by 1/tau
// every Tupdate, multiply state by exp(-Tupdate/tau)
//
// state value is the spike rate in Hz
// let's say the max spike rate is 100KHz. Overkill, but we'll spare the bits
//
// a reasonable 27b fixed-point state representation is then 18.9 (max 128K)
// decay_constants are < 1, so we'll use 0.27
//
// multiplication returns a 54b output, 18.36. We discard the lower 27 bits
// we can test the MSB, if the rate is over 64K make we should emit a warning
  
////////////////////////////////////////////
// state update logic

logic [Nfilts-1:0] last_filt_idx, filt_idx, next_filt_idx, next_filt_idx_p1;
enum {READY, INCREMENT_1, INCREMENT_2, FIRST_UPDATE, UPDATE, LAST_UPDATE} state, next_state;

assign next_filt_idx_p1 = next_filt_idx + 1;

always_ff @(posedge clk, posedge reset)
  if (reset == 1) begin
    state <= READY;
    filt_idx <= 0;
    last_filt_idx <= 'X;
  end
  else begin
    state <= next_state;
    filt_idx <= next_filt_idx;
    last_filt_idx <= filt_idx;
  end

// next_state computation
always_comb
  unique case (state)

    READY:
      // waiting for a tag or the update pulse
      if (in.v == 1) begin
        next_state = INCREMENT_1;
        next_filt_idx = in.tag; // XXX could use a different register for this
      end
      else if (update_pulse == 1) begin
        next_state = FIRST_UPDATE;
        next_filt_idx = 0;
      end
      else begin
        next_state = READY;
        next_filt_idx = 'X;
      end

    INCREMENT_1: begin
      // tag arrived, read memory, write next cycle
      next_state = INCREMENT_2;
      next_filt_idx = 'X;
    end

    INCREMENT_2: begin
      // write increment results after receiving tag
      next_state = READY;
      next_filt_idx = 'X;
    end

    FIRST_UPDATE: begin
      // read from filt_idx (= 0), no write
      assert (update_pulse == 0); // failing means we aren't fast enough to update every filter in one time unit
      if (out.r) 
        next_filt_idx = filt_idx_p1;
        next_state = UPDATE;
      else
        next_filt_idx = filt_idx;
        next_state = FIRST_UPDATE;
    end

    UPDATE: begin
      assert (update_pulse == 0); // failing means we aren't fast enough to update every filter in one time unit
      // we are in this state for Nfilt - 1 cycles
      // read from filt_idx and write to last_filt_idx
      if (out.r) begin
        next_filt_idx = filt_idx_p1;
        if (filt_idx_p1 < filts_used) // note _p1
          next_state = UPDATE;
        else
          next_state = LAST_UPDATE;
      end
      else begin
        next_filt_idx = filt_idx;
        next_state = UPDATE;
      end
    end

    LAST_UPDATE: begin
      // write to last_filt_idx
      // don't have to worry about out.r, nothing to send
      assert (update_pulse == 0); // failing means we aren't fast enough to update every filter in one time unit
      next_state = READY;
      next_filt_idx = 0;
    end
      
  endcase

////////////////////////////////////////////
// memory read/write logic
logic mem_rd_en;
logic [Nfilt-1:0] mem_rd_addr;
logic [Nct-1:0] mem_rd_data;

logic mem_wr_en;
logic [Nfilt-1:0] mem_wr_addr;
logic [Nct-1:0] mem_wr_data;

// combinational logic for increment
// meant to synthesize as a DSP multiplier
// multiply the input count with the increment_constant
// discard the high-order bits
// could generate a warning if any of the high bits are non-zero
logic [(Nstate+Nct)-1:0] increment_mult_out;
logic [Nstate-1:0] increment_amount;
logic [Nstate-1:0] increment_writeback;
assign increment_mult_out = increment_constant * in.ct;
assign increment_amount = increment_mult_out[Nstate-1:0]; // discard MSBs (Nct.0 * Nstate-9.9)
assign increment_writeback = mem_rd_data + increment_amount;

// combinational block for update
// meant to synthesize as a DSP multiplier
// multiple the state with the decay_constant
logic [(2*Nstate)-1:0] update_mult_out;
logic [Nstate-1:0] update_writeback;
assign update_mult_out = decay_constant * mem_rd_data;
assign update_writeback = update_mult_out[(2*Nstate)-1:Nstate]; // discard LSBs (see above, 0.Nstate * Nstate-9.9)

always_comb
  unique case (state)

    READY: begin
      // no read or write
      mem_rd_en = 0;
      mem_rd_addr = 'X;

      mem_wr_en = 0;
      mem_wr_addr = 'X;
      mem_wr_data = 'X;
    end

    INCREMENT_1: begin
      // do read, no write yet
      mem_rd_en = 1;
      mem_rd_addr = filt_idx;

      mem_wr_en = 0;
      mem_wr_addr = 'X;
      mem_wr_data = 'X;
    end

    INCREMENT_2: begin
      // no read, do write
      mem_rd_en = 0;
      mem_rd_addr = 'X;

      mem_wr_en = 1;
      mem_wr_addr = last_filt_idx;
      mem_wr_data = increment_writeback;
    end

    FIRST_UPDATE: begin
      // read from filt_idx (= 0), no write
      mem_rd_en = 1;
      mem_rd_addr = filt_idx;

      mem_wr_en = 0;
      mem_wr_addr = 'X;
      mem_wr_data = 'X;
    end

    UPDATE: begin
      // read from filt_idx and write to last_filt_idx
      mem_rd_en = 1;
      mem_rd_addr = filt_idx;

      mem_wr_en = 1;
      mem_wr_addr = last_filt_idx;
      mem_wr_data = update_writeback;
    end

    LAST_UPDATE: begin
      mem_rd_en = 0;
      mem_rd_addr = 'X;

      mem_wr_en = 1;
      mem_wr_addr = last_filt_idx;
      mem_wr_data = update_writeback;
    end
  endcase

////////////////////////////////////////////
// in.a logic, don't handshake until INCREMENT_2 to hold in.ct
assign in.a = (state == INCREMENT_2);

////////////////////////////////////////////
// output logic
always_comb 
  if (state == UPDATE || state == LAST_UPDATE) begin
    out.state = mem_rd_data;
    out.filt = last_filt_idx;
    out.v = 1;
  end
  else
    out.state = 'X;
    out.filt = 'X;
    out.v = 0;
  end




