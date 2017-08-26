`include "SpikeFilterMem.v"
`include "Interfaces.svh"

// low-pass filters a variable number of spike streams
// uses a decaying exponential filter
// implemented as a pipeline, one update/clk
module SpikeFilterArray #(parameter Nfilts = 10, parameter Nstate = 27, parameter Nct = 10) ( // 27 is the width of the cyclone 5's high-precision DSP block
  FilterOutputChannel out, 

  TagCtChannel in, 

  input update_pulse, // prompts all filters to update their values

  SpikeFilterConf conf,

  input clk,
  input reset);

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
// conf.decay_constants are < 1, so we'll use 0.27
//
// multiplication returns a 54b output, 18.36. We discard the lower 27 bits
// we can test the MSB, if the rate is over 64K make we should emit a warning
//
// e.g: 
// tau = 100 ms = .1 s, Tupdate = 100us = .0001s
//
// conf.increment_constant = 1/.1 = 10
//   2**9 * 10 = 5120
//
// conf.decay_constant = exp(-.0001/.1) ~ .9990005
//   exp(-.0001/.1) * 2**27 = 134083577

////////////////////////////////////////////
// pipeline structure
// note that the memory has an input register, which complicates things
//
//        s1               s2               s3              s4             s5               
//       state    /    pipe input     /  mem read   /    writeback    / mem write 
//                                                                      
//                        in                                            
//                         |                        +-{inc}-+              
//    next_state  state    |                        |       |              
// [FSM]------[*]---+---{logic}--[-[*]-mem]--m--[*]-+     {mux}--[-[*]-[mem]]
//   |              |      |                 |      |       |              
//   +--------------+      +-------[*]-------+      +-{dec}-+              
//                                                      |                  
//                             ("m" for merge)          +----------[*]-- out    
//                                                                      
// (out.v & ~out.a) is treated as stall for all registers

// instantiate memory, declare ports
logic mem_rd_en;
logic [Nfilts-1:0] mem_rd_addr;
logic [Nstate-1:0] mem_rd_data;
logic mem_wr_en;
logic [Nfilts-1:0] mem_wr_addr;
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
enum {READY_OR_INCREMENT, PRE_BUBBLE2, PRE_BUBBLE1, DECAY_UPDATE, BUBBLE2, BUBBLE1} state, next_state;
logic saw_update_pulse, next_saw_update_pulse; // record if we got the update pulse while BUBBLEing

assign filt_idx_p1 = filt_idx + 1;

logic stall;
assign stall = out.v & ~out.a;

always_ff @(posedge clk, posedge reset)
  if (reset == 1) begin
    state <= READY_OR_INCREMENT;
    filt_idx <= 0;
    saw_update_pulse <= 0;
  end
  else
    if (stall == 1) begin
      state <= state;
      filt_idx <= filt_idx;
      saw_update_pulse <= saw_update_pulse;
    end
    else begin
      state <= next_state;
      filt_idx <= next_filt_idx;
      saw_update_pulse <= next_saw_update_pulse;
    end

// next_state computation
always_comb
  unique case (state)

    READY_OR_INCREMENT: begin
      // waiting for a tag or the update pulse
      if (in.v == 1) begin
        if (update_pulse == 1 || saw_update_pulse == 1)
          next_saw_update_pulse <= 1;
        next_state <= READY_OR_INCREMENT;
        next_filt_idx = 'X;
      end
      else if (update_pulse == 1 || saw_update_pulse == 1) begin
        next_saw_update_pulse <= 0;
        next_state = PRE_BUBBLE2;
        next_filt_idx = 'X;
      end
      else begin
        next_saw_update_pulse <= 0;
        next_state = READY_OR_INCREMENT;
        next_filt_idx = 'X;
      end
    end

    DECAY_UPDATE: begin
      assert (update_pulse == 0);
      next_saw_update_pulse <= 0;
      if (filt_idx_p1 < conf.filts_used) begin
        next_state = DECAY_UPDATE;
        next_filt_idx = filt_idx_p1;
      end
      else begin
        next_state = BUBBLE2;
        next_filt_idx = 'X;
      end
    end

    // need to bubble for two cycles to avoid pipeline hazard:
    // it's possible that we do an increment immediately following decay,
    // causing the value to be read BEFORE it's decayed and written back
    // the subsequent write clobbers the decay
    // same thing can happen with decay following increment
    BUBBLE2: begin
      next_state = BUBBLE1;
      next_filt_idx = 'X;
      if (update_pulse == 1 || saw_update_pulse == 1)
        next_saw_update_pulse <= 1;
    end

    BUBBLE1: begin
      next_state = READY_OR_INCREMENT;
      next_filt_idx = 'X;
      if (update_pulse == 1 || saw_update_pulse == 1)
        next_saw_update_pulse <= 1;
    end

    PRE_BUBBLE2: begin
      next_state = PRE_BUBBLE1;
      next_filt_idx = 'X;
      next_saw_update_pulse <= 0;
    end

    PRE_BUBBLE1: begin
      next_state = DECAY_UPDATE;
      next_filt_idx = 0; // set to 0
      next_saw_update_pulse <= 0;
    end

  endcase

////////////////////////////////////////////
// pipeline data, use consistent naming for the rest of the pipeline
//
// stage N contains some combinational logic followed by a register
//
// sN_* refers to the output of stage N, the output of it's register
// sN_next_* refers to the input to stage N's register, the output next cycle
//
// Some of these are redundant or unecessary because the memory registers its
// inputs, but they are retained for clarity.
// Note that the memory's inputs are driven off of the sN_next_* signals

// filter idx/memory address
logic [Nfilts-1:0] s2_filt_idx, s2_next_filt_idx, s3_filt_idx, s3_next_filt_idx, s4_filt_idx, s4_next_filt_idx;
// filter state
logic [Nstate-1:0] s3_next_filt_state, s3_filt_state, s4_next_filt_state, s4_filt_state;
// opcodes
enum {INCREMENT, DECAY, NOP} s2_op, s2_next_op, s3_op, s3_next_op, s4_op, s4_next_op;
// count, for INCREMENT op
logic [Nct-1:0] s2_ct, s2_next_ct, s3_ct, s3_next_ct;

// pipeline register updates
always_ff @(posedge clk, posedge reset)
  if (reset == 1) begin
    s2_op <= NOP;
    s3_op <= NOP;
    s4_op <= NOP;
    s2_filt_idx <= 'X;
    s3_filt_idx <= 'X;
    s4_filt_idx <= 'X;
    s2_ct <= 'X;
    s3_ct <= 'X;
    s4_filt_state <= 'X;
  end
  else 
    if (stall == 1) begin
      s2_op <= s2_op;
      s3_op <= s3_op;
      s4_op <= s4_op;
      s2_filt_idx <= s2_filt_idx;
      s3_filt_idx <= s3_filt_idx;
      s4_filt_idx <= s4_filt_idx;
      s2_ct <= s2_ct;
      s3_ct <= s3_ct;
      s3_filt_state <= s3_filt_state;
      s4_filt_state <= s4_filt_state;
    end
    else begin
      s2_op <= s2_next_op;
      s3_op <= s3_next_op;
      s4_op <= s4_next_op;
      s2_filt_idx <= s2_next_filt_idx;
      s3_filt_idx <= s3_next_filt_idx;
      s4_filt_idx <= s4_next_filt_idx;
      s2_ct <= s2_next_ct;
      s3_ct <= s3_next_ct;
      s3_filt_state <= s3_next_filt_state;
      s4_filt_state <= s4_next_filt_state;
    end

////////////////////////////////////////////
// stage 2: state output/pipeline input

logic do_inc;
assign do_inc = (state == READY_OR_INCREMENT) & in.v & (in.tag < conf.filts_used);

always_comb
  if (do_inc == 1) begin
    s2_next_op = INCREMENT;
    s2_next_filt_idx = in.tag;
    s2_next_ct = in.ct;
  end
  else if (state == DECAY_UPDATE) begin
    s2_next_op = DECAY;
    s2_next_filt_idx = filt_idx;
    s2_next_ct = 'X;
  end
  else begin
    s2_next_op = NOP;
    s2_next_filt_idx = 'X;
    s2_next_ct = 'X;
  end

// handshake input (maybe this can be improved, depends on last stage)
// this is almost do_inc, but we handshake anyway if we get an input >= conf.filts_used
assign in.a = (state == READY_OR_INCREMENT) & in.v & ~stall;

////////////////////////////////////////////
// stage 3: memory read

// memory inputs 
// NOTE: driven by s2_next_* instead of s2_* because of registered inputs
assign mem_rd_en = (s2_next_op == DECAY || s2_next_op == INCREMENT);
assign mem_rd_addr = s2_next_filt_idx;

// memory output
assign s3_next_filt_state = mem_rd_data;

// passthrough
assign s3_next_op = s2_op;
assign s3_next_filt_idx = s2_filt_idx;
assign s3_next_ct = s2_ct;

////////////////////////////////////////////
// stage 4: memory writeback computation, output

// combinational logic for INCREMENT
// meant to synthesize as a DSP multiplier
// multiply the count with the conf.increment_constant
// discard the high-order bits
// could generate a warning to the user if any of the high bits are non-zero
logic [(Nstate+Nct)-1:0] increment_mult_out;
logic [Nstate-1:0] increment_amount;
logic [Nstate-1:0] increment_writeback;
assign increment_mult_out = conf.increment_constant * s3_ct;
assign increment_amount = increment_mult_out[Nstate-1:0]; // discard MSBs (Nct.0 * Nstate-9.9)
assign increment_writeback = s3_filt_state + increment_amount;

// combinational block for DECAY
// meant to synthesize as a DSP multiplier
// multiple the state with the conf.decay_constant
logic [(2*Nstate)-1:0] decay_mult_out;
logic [Nstate-1:0] decay_writeback;
assign decay_mult_out = conf.decay_constant * s3_filt_state;
assign decay_writeback = decay_mult_out[(2*Nstate)-1:Nstate]; // discard LSBs (see above, 0.Nstate * Nstate-9.9)

// mux INCREMENT and DECAY writebacks
always_comb
  unique case (s3_op)
    INCREMENT:
      s4_next_filt_state = increment_writeback;
    DECAY:
      s4_next_filt_state = decay_writeback;
    NOP:
      s4_next_filt_state = 'X;
  endcase

// passthrough
assign s4_next_filt_idx = s3_filt_idx;
assign s4_next_op = s3_op;
// no more ct

// drive outputs if DECAY and output is ready
always_comb
  if (s3_op == DECAY) begin
    out.v = 1;
    out.filt_state = s3_filt_state;
    out.filt_idx = s3_filt_idx;
  end
  else begin
    out.v = 0;
    out.filt_state = 'X;
    out.filt_idx = 'X;
  end

////////////////////////////////////////////
// stage 5: memory write

// NOTE driven off s4_next_* instead of s4_* because of input register
assign mem_wr_en = (s4_next_op == INCREMENT || s4_next_op == DECAY);
assign mem_wr_addr = s4_next_filt_idx;
assign mem_wr_data = s4_next_filt_state;

endmodule

////////////////////////////////////////////
// TESTBENCH
module SpikeFilterArray_tb;

parameter Nfilts = 10;
parameter Nstate = 27;
parameter Nct = 10;

FilterOutputChannel out();

TagCtChannel #(.Ntag(Nfilts), .Nct(Nct)) in();

SpikeFilterConf #(Nfilts, Nstate) conf();

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

// update_pulse
parameter ClksPerUpdate = 64;
logic update_pulse;
initial begin
  update_pulse <= 0;
  forever begin
    #(ClksPerUpdate*Tclk) @(posedge clk) update_pulse <= 1;
    @(posedge clk) update_pulse <= 0;
  end
end

// receiver
Channel #(Nstate + Nfilts) packed_out();
assign packed_out.v = out.v;
assign packed_out.d = {out.filt_idx, out.filt_state};
assign out.a = packed_out.a;
ChannelSink #(.ClkDelaysMin(0), .ClkDelaysMax(10)) out_sink(packed_out, clk, reset);

// stimulus
initial begin
  @(posedge reset)
  // "count mode"
  conf.increment_constant <= 1;
  conf.decay_constant <= 0;

  // filts initially off
  conf.filts_used <= 0;

  // send some tags, make sure nothing happens
  #(Tclk * 10) 
  @(posedge clk) 
  in.v <= 1;
  in.tag <= 0;
  in.ct <= 1;
  @(posedge clk) 
  in.v <= 0;
  in.tag <= 'X;
  in.ct <= 'X;

  #(Tclk * 10) 
  @(posedge clk) 
  in.v <= 1;
  in.tag <= 1;
  in.ct <= 1;
  @(posedge clk) 
  in.v <= 0;
  in.tag <= 'X;
  in.ct <= 'X;

  // turn 2 filts on
  conf.filts_used <= 2;

  // send tags
  repeat(10) begin
    #(Tclk * 10) 
    @(posedge clk);
    in.v <= 1;
    in.tag <= 0;
    in.ct <= 1;
    forever begin 
      @(posedge clk);
      if (in.a == 1) begin
        in.v <= 0;
        in.tag <= 'X;
        in.ct <= 'X;
        break;
      end
    end

    #(Tclk * 10) 
    @(posedge clk);
    in.v <= 1;
    in.tag <= 1;
    in.ct <= 2;
    forever begin 
      @(posedge clk);
      if (in.a == 1) begin
        in.v <= 0;
        in.tag <= 'X;
        in.ct <= 'X;
        break;
      end
    end
  end

  // do the same thing in "decay mode"
  // using numbers from example
  #(Tclk * 10) 
  conf.increment_constant <= 5120;
  conf.decay_constant <= 134083577;

  // send tags
  repeat(10) begin
    #(Tclk * 10) 
    @(posedge clk);
    in.v <= 1;
    in.tag <= 0;
    in.ct <= 1;
    forever begin 
      @(posedge clk);
      if (in.a == 1) begin
        in.v <= 0;
        in.tag <= 'X;
        in.ct <= 'X;
        break;
      end
    end

    #(Tclk * 10) 
    @(posedge clk);
    in.v <= 1;
    in.tag <= 1;
    in.ct <= 2;
    forever begin
      @(posedge clk);
      if (in.a == 1) begin
        in.v <= 0;
        in.tag <= 'X;
        in.ct <= 'X;
        break;
      end
    end
  end

end

SpikeFilterArray dut(.*);

endmodule
