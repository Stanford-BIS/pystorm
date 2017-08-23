`include "SpikeFilterMem.v"
`include "Interfaces.svh"

// low-pass filters a variable number of spike streams
// uses a decaying exponential filter
// implemented as a pipeline, one update/clk
module SpikeFilterArray #(parameter Nfilts = 10, parameter Nstate = 27, parameter Nct = 10) ( // 27 is the width of the cyclone 5's high-precision DSP block
  FilterOutputChannel out, 

  TagCtChannel in, 

  input update_pulse, // prompts all filters to update their values
  input [Nfilts-1:0] filts_used,

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
// pipeline structure
//
//        s1               s2              s3          s4           s5               
//       state    /    pipe input    /  mem read /  writeback  / mem write 
//                                                                      
//                        in                                            
//                         |                     +-{inc}-+              
//    next_state  state    |                     |       |              
// [FSM]------[*]---+---{logic}--[*]--[mem]--[*]-+     {mux}-[*]--[mem]  
//   |              |                            |       |              
//   +--------------+                            +-{dec}-+              
//                                                   |                  
//                                                   +-----[*]-- out    
//                                                                      
// out.r is treated as ~stall for all registers

// instantiate memory, declare ports
logic mem_rd_en;
logic [Nfilt-1:0] mem_rd_addr;
logic [Nstate-1:0] mem_rd_data;
logic mem_wr_en;
logic [Nfilt-1:0] mem_wr_addr;
logic [Nstate-1:0] mem_wr_data;

SpikeFilterMem mem(
  .wraddress(mem_wr_addr),
  .data(mem_wr_data),
  .wren(mem_wr_en),
  .rdaddress(mem_rd_addr),
  .q(mem_rd_data),
  .rden(mem_rd_en),
  .clock(clk));

////////////////////////////////////////////
// stage 1: state

logic [Nfilts-1:0] filt_idx, next_filt_idx, filt_idx_p1;
enum {READY_OR_INCREMENT, DECAY_UPDATE} state, next_state;

assign filt_idx_p1 = filt_idx + 1;

always_ff @(posedge clk, posedge reset)
  if (reset == 1) begin
    state <= READY_OR_INCREMENT;
    filt_idx <= 0;
  end
  else begin
    state <= next_state;
    filt_idx <= next_filt_idx;
  end

// next_state computation
always_comb
  if (out.r == 0) begin
    next_state <= state;
    next_filt_idx <= filt_idx;
  end
  else
    unique case (state)

      READY_OR_INCREMENT:
        // waiting for a tag or the update pulse
        else if (update_pulse == 1) begin
          next_state = DECAY_UPDATE;
          next_filt_idx = 0;
        end
        else begin
          next_state = READY_OR_INCREMENT;
          next_filt_idx = in.tag; 
        end

      DECAY_UPDATE:
        if (filt_idx_p1 < filts_used) begin
          next_state = DECAY_UPDATE;
          next_filt_idx = filt_idx_p1;
        end
        else begin
          next_state = READY_OR_INCREMENT;
          next_filt_idx = 'X;
        end

      endcase

////////////////////////////////////////////
// pipeline data, use consistent naming for the rest of the pipeline
//
// stage N contains some combinational logic followed by a register
//
// sN_* refers to the output of stage N, the output of it's register
// sN_next_* refers to the input to stage N's register, the output next cycle

// filter idx/memory address
logic [Nfilts-1:0] s2_filt_idx, s2_next_filt_idx, s3_filt_idx, s3_next_filt_idx, s4_filt_idx, s4_next_filt_idx;
// filter state
logic [Nstate-1:0] s3_next_state, s3_state, s4_next_state, s4_state;
// opcodes
enum {INCREMENT, DECAY, NOP} s2_op, s2_next_op, s3_op, s3_next_op, s4_op, s4_next_op;
// count, for INCREMENT op
logic [Nct-1:0] s2_ct, s2_next_ct, s3_ct, s3_next_ct;

// pipeline register updates
always_ff @(posedge clk, posedge reset)
  if (reset == 0)
    s2_op <= NOP;
    s3_op <= NOP;
    s4_op <= NOP;
    s2_filt_idx <= 'X;
    s3_filt_idx <= 'X;
    s4_filt_idx <= 'X;
    s2_ct <= 'X;
    s3_ct <= 'X;
    s4_state <= 'X;
  else
    if (out.r == 1) begin // out.r is our inverted stall condition
      s2_op <= s2_next_op 
      s3_op <= s3_next_op 
      s4_op <= s4_next_op 
      s2_filt_idx <= s2_next_filt_idx 
      s3_filt_idx <= s3_next_filt_idx 
      s4_filt_idx <= s4_next_filt_idx 
      s2_ct <= s2_next_ct 
      s3_ct <= s3_next_ct 
      s3_state <= s3_next_state;
      s4_state <= s4_next_state;
    end
    else begin
      s2_op <= s2_op 
      s3_op <= s3_op 
      s4_op <= s4_op 
      s2_filt_idx <= s2_filt_idx 
      s3_filt_idx <= s3_filt_idx 
      s4_filt_idx <= s4_filt_idx 
      s2_ct <= s2_ct 
      s3_ct <= s3_ct 
      s3_state <= s3_state;
      s4_state <= s4_state;
    end

////////////////////////////////////////////
// stage 2: state output/pipeline input

always_comb
  unique case (state)

    READY_OR_INCREMENT:
      if (in.v == 1) begin
        s2_next_op = INCREMENT;
        s2_next_filt_idx = in.tag;
        s2_ct = in.ct;
      end
      else begin
        s2_next_op = NOP;
        s2_next_filt_idx = 'X;
        s2_next_ct = 'X;
      end

    DECAY_UPDATE: begin 
      s2_next_op = DECAY;
      s2_next_filt_idx = filt_idx;
      s2_next_ct = 'X;

  endcase

// handshake input
assign in.r = out.r;

////////////////////////////////////////////
// stage 3: memory read

// memory inputs
assign mem_rd_en = (s2_op != NOP);
assign mem_rd_addr = s2_filt_idx;

// memory output
assign s3_next_state = mem_rd_data;

// passthrough
assign s3_next_op = s2_op;
assign s3_next_filt_idx = s2_filt_idx;
assign s3_next_ct = s2_ct;

////////////////////////////////////////////
// stage 4: memory writeback computation, output

// combinational logic for INCREMENT
// meant to synthesize as a DSP multiplier
// multiply the count with the increment_constant
// discard the high-order bits
// could generate a warning to the user if any of the high bits are non-zero
logic [(Nstate+Nct)-1:0] increment_mult_out;
logic [Nstate-1:0] increment_amount;
logic [Nstate-1:0] increment_writeback;
assign increment_mult_out = increment_constant * s3_ct;
assign increment_amount = increment_mult_out[Nstate-1:0]; // discard MSBs (Nct.0 * Nstate-9.9)
assign increment_writeback = s3_state + increment_amount;

// combinational block for DECAY
// meant to synthesize as a DSP multiplier
// multiple the state with the decay_constant
logic [(2*Nstate)-1:0] decay_mult_out;
logic [Nstate-1:0] decay_writeback;
assign decay_mult_out = decay_constant * s3_state;
assign decay_writeback = decay_mult_out[(2*Nstate)-1:Nstate]; // discard LSBs (see above, 0.Nstate * Nstate-9.9)

// mux INCREMENT and DECAY writebacks
always_comb
  unique case (s3_op)
    INCREMENT:
      s4_next_state = increment_writeback;
    DECAY:
      s4_next_state = decay_writeback;
    NOP:
      s4_next_state = 'X;

// passthrough
s4_next_filt_idx = s3_filt_idx;
s4_next_op = s3_op;
// no more ct

// drive outputs if DECAY
always_comb
  if (s3_op == DECAY) begin
    out.v = 1;
    out.state = s3_state;
    out.filt = s3_filt_idx;
  end
  else begin
    out.v = 0;
    out.state = 'X;
    out.filt = 'X;
  end

////////////////////////////////////////////
// stage 5: memory write

assign mem_wr_en = (s4_op != NOP);
assign mem_wr_addr = s4_filt_idx;
assign mem_wr_data = s4_state;


////////////////////////////////////////////
// TESTBENCH

