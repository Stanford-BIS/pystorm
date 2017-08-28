`include "Interfaces.svh"
`include "Channel.svh"

// PCPacker collects the two upwards streams,
// the BD traffic, and the SpikeGeneratorArray traffic,
// and inserts heartbeat events signalsed by the TimeMgr

module PCPacker #(
  parameter NPCout = 40,
  parameter NBDdata = 34,
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
//////////////////////////////////////////////
// Word from BD
//
//  MSB              LSB
//   1   5       34
// [ 0 | X | BD payload ]
//
// MSB = 0 means from BD
//
//////////////////////////////////////////////
// FPGA-filtered spike stream
//
//  MSB                    LSB
//   1   1      11       27
// [ 1 | 0 | filt_idx | state ]
//
// MSB = 1 means from FPGA
// MSB-1 = 0 means spike stream
// <filt_idx> is code for which spike filter <state> corresponds to 
// <state> is the state of the spike filter
//
// (note: 27 bits is unlikely to change, it's the DSP width,
//  11 is just the bits remaining, there may not be 2**11 filters)
//
//////////////////////////////////////////////
// FPGA-generated heartbeat
// (split into two packets)
//
//  MSB                      LSB
//   1   1    14         24 
// [ 1 | 1 | hi_lo |  time_bits ]
//
// MSB = 1 means from FPGA
// MSB-1 = 1 means heartbeat
// <hi_lo> is 0 or 1, 0 for time LSBs, 1 for time MSBs
// <time_bits> is the time value (MSBs or LSBs), in time units

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
MergeChannels packed_merge_0(packed_merge_0_out, SF_packed, HB_packed, clk, reset);
MergeChannels packed_merge_root(PC_out, BD_packed, packed_merge_0.out, clk, reset);

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
