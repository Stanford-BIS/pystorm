`include "Interfaces.svh"
`include "Channel.svh"

// PCPacker collects the two upwards streams,
// the BD traffic, and the SpikeGeneratorArray traffic,
// and inserts heartbeat events signalsed by the TimeMgr

module PCPacker #(
  parameter NPCout = 32,
  parameter NBDdata = 16,
  parameter Ntime = 48,
  parameter N_SF_filts = 10,
  parameter N_SF_state = 27) (
  // our only output goes to the PC
  Channel PC_out,

  // BD inputs, passthrough
  Channel BD_in,

  // TimeMgr inputs
  input send_HB_up_pulse, // pulses every send_HB_up_every * time_unit, for 1 clk cycle
  input [Ntime-1:0] time_elapsed, // current wall time, can pack into unused spike bits for higher resolution
   
  // Filtered spikes
  SpikeFilterOutputChannel SF_in,

  input clk, reset);

//////////////////////////////////////////////
// FPGA packet formats
//
// All words look like this:
//
//    4      12
// [ code | data ]
//
// FPGA and BD codes share the same 4-bit space,
// there's no FPGA/BD bit
//
//////////////////////////////////////////////
// Word from BD
//
//  MSB            LSB
//    4            12
// [ code | BD_payload_chunk ]
//
// BD words use codes 0-12 (13 funnel leaves)
// most words get serialized, LSBs first
//
// (if we want to free up some codes, could combine
//  similar streams into new word formats. e.g. the overflows
//  or FIFO dumps could be combined into a single code)
//
//////////////////////////////////////////////
// FPGA-filtered spike/tag stream
//
// 4 words per event:
//
//  MSB                  LSB
//      4           12
// [ code=14 |   filt_idx   ]
// 
//      4           12
// [ code=14 | state[11:0]  ]
//
//      4           12
// [ code=14 | state[23:12] ]
//
//      4           12
// [ code=14 | state[26:24] ]
//
// (note: 27 state bits is unlikely to change, it's the DSP width,
//  12 is just the bits remaining for filt_idx, there may not be 2**12 filters)
//
//////////////////////////////////////////////
// FPGA-generated heartbeat
// (split into two packets)
//
//  MSB                                   LSB
//      4                    12 
// [ code=15 |  time_bits[(12*(N+1))-1:12*N] ]
//

// turn all the inputs into their PC_out packet formats then merge them
Channel #(NPCout) BD_packed();
Channel #(NPCout) HB_packed();
Channel #(NPCout) SF_packed();

//////////////////////////////////////////////
// BD outputs

parameter N_BD_packed_X = NPCout - NBDdata - 1;
const logic [N_BD_packed_X-1:0] BD_packed_X = '0;

assign BD_packed.v = BD_in.v;
assign BD_packed.d = {1'b0, BD_packed_X, BD_in.d};
assign BD_in.a = BD_packed.a;

//////////////////////////////////////////////
// Heartbeat
// needs a little FSM to latch the HB and send the time_elapsed halves
// has an extra cycle of delay for simplicity

enum {READY, SENDING_LO, SENDING_HI} send_HB_state, next_send_HB_state;
always_ff @(posedge clk, posedge reset)
  if (reset == 1)
    send_HB_state <= READY;
  else
    send_HB_state <= next_send_HB_state;

always_comb
  unique case (send_HB_state)
    READY:
      if (send_HB_up_pulse == 1)
        next_send_HB_state = SENDING_LO;
      else
        next_send_HB_state = READY;
    SENDING_LO:
      if (HB_packed.a == 1)
        next_send_HB_state = SENDING_HI;
      else
        next_send_HB_state = SENDING_LO;
    SENDING_HI:
      if (HB_packed.a == 1)
        next_send_HB_state = READY;
      else
        next_send_HB_state = SENDING_HI;
  endcase

parameter N_HB_packed_hi_lo = NPCout - Ntime/2 - 2;
const logic [N_HB_packed_hi_lo-1:0] HB_lo = 0;
const logic [N_HB_packed_hi_lo-1:0] HB_hi = 1;

always_comb
  unique case (send_HB_state)
    READY: begin
      HB_packed.v = 0;
      HB_packed.d = 'X;
    end
    SENDING_LO: begin
      HB_packed.v = 1;
      HB_packed.d = {1'b1, 1'b1, HB_lo, time_elapsed[Ntime/2-1:0]}; // LSBs
    end
    SENDING_HI: begin
      HB_packed.v = 1;
      HB_packed.d = {1'b1, 1'b1, HB_hi, time_elapsed[Ntime-1:Ntime/2]}; // MSBs
    end
  endcase

//////////////////////////////////////////////
// SpikeFilter

logic [(NPCout - N_SF_state - 2)-1:0] filt_idx_extended;
assign filt_idx_extended = SF_in.filt_idx; // zero extends by default
assign SF_packed.v = SF_in.v;
assign SF_packed.d = {1'b1, 1'b0, filt_idx_extended, SF_in.filt_state};
assign SF_in.a = SF_packed.a;

//////////////////////////////////////////////
// Merge *_packed channels

Channel #(NPCout) packed_merge_0_out();
ChannelMerge packed_merge_0(packed_merge_0_out, SF_packed, HB_packed, clk, reset);
ChannelMerge packed_merge_root(PC_out, BD_packed, packed_merge_0.out, clk, reset);

endmodule

//////////////////////////////////////////////
// TESTBENCH
module PCPacker_tb;

parameter NPCout = 40;
parameter NBDdata = 34;
parameter Ntime = 48;
parameter N_SF_filts = 10;
parameter N_SF_state = 27;

// our only output goes to the PC
Channel #(NPCout) PC_out();

// BD inputs; passthrough
Channel #(NBDdata) BD_in();
 
// Filtered spikes
SpikeFilterOutputChannel #(N_SF_filts, N_SF_state) SF_in();

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

// HB pulse source
logic send_HB_up_pulse; // pulses every send_HB_up_every * time_unit; for 1 clk cycle 
logic [Ntime-1:0] time_elapsed; // current wall time; can pack into unused spike bits for higher resolution

parameter ClksPerHB = 64;
initial begin
  send_HB_up_pulse <= 0;
  time_elapsed <= 0; // we're just going to count pulses
  forever begin
    #(ClksPerHB*Tclk) @(posedge clk) send_HB_up_pulse <= 1;
    @(posedge clk);
    send_HB_up_pulse <= 0;
    time_elapsed <= time_elapsed + 1;
  end
end

// SF sender
Channel #(N_SF_filts + N_SF_state) SF_in_packed();
assign SF_in_packed.a = SF_in.a;
assign {SF_in.filt_idx, SF_in.filt_state} = SF_in_packed.d;
assign SF_in.v = SF_in_packed.v;
RandomChannelSrc #(.N(N_SF_filts + N_SF_state), .ClkDelaysMin(0), .ClkDelaysMax(10)) SF_in_src(SF_in_packed, clk, reset);

// BD sender
RandomChannelSrc #(.N(NBDdata), .ClkDelaysMin(0), .ClkDelaysMax(10)) BD_in_src(BD_in, clk, reset);

// output sink
ChannelSink #(.ClkDelaysMin(0), .ClkDelaysMax(2)) PC_out_sink(PC_out, clk, reset);

PCPacker dut(.*);

endmodule
