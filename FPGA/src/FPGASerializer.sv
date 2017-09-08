`include "Channel.svh"
`include "Interfaces.svh"

module FPGASerializer #(
  parameter NPCcode = 8,
  parameter NPCdata = 24,
  parameter Ntime = 48,
  parameter N_SF_filts = 10,
  parameter N_SF_state = 27) (

  // our only output goes to the PC
  SerializedPCWordChannel PC_out,

  // TimeMgr inputs
  input send_HB_up_pulse, // pulses every send_HB_up_every * time_unit, for 1 clk cycle
  input [Ntime-1:0] time_elapsed, // current wall time, can pack into unused spike bits for higher resolution
   
  // Filtered spikes
  SpikeFilterOutputChannel SF_in,

  input clk, reset);

localparam logic [NPCcode-1:0] HB_code = 13;
localparam logic [NPCcode-1:0] SF_code = 14;

// 1. pack/convert inputs into Channels
// 2. serialize, 
// 3. append codes
// 4. merge
// 
// could make this more regular by forming a ChannelArray of pre-packed streams
// doing step 2,3 in a loop
// for now there are only 2 inputs, though

Channel #(Ntime) HB_packed();
Channel #(N_SF_filts + N_SF_state) SF_packed();

Channel #(NPCdata) HB_serialized();
Channel #(NPCdata) SF_serialized();

Channel #(NPCcode + NPCdata) HB_coded();
Channel #(NPCcode + NPCdata) SF_coded();

//////////////////////////////////////////////
// Heartbeat

// pulse -> channel
PulseToChannel #(Ntime) HB_pulse_to_channel(HB_packed, time_elapsed, send_HB_up_pulse, clk, reset);

// serialize
Serializer #(.Nin(Ntime), .Nout(NPCdata)) HB_serializer(HB_serialized, HB_packed, clk, reset);

// append code
assign HB_coded.d = {HB_code, HB_serialized.d}; // will 0-extend
assign HB_coded.v = HB_serialized.v;
assign HB_serialized.a = HB_coded.a;

//////////////////////////////////////////////
// SpikeFilter

// pack
assign SF_packed.d = {SF_in.filt_idx, SF_in.filt_state};
assign SF_packed.v = SF_in.v;
assign SF_in.a = SF_packed.a;

// serialize
Serializer #(.Nin(N_SF_filts + N_SF_state), .Nout(NPCdata)) SF_serializer(SF_serialized, SF_packed, clk, reset);

// append code
assign SF_coded.d = {SF_code, SF_serialized.d}; // will 0-extend
assign SF_coded.v = SF_serialized.v;
assign SF_serialized.a = SF_coded.a;

//////////////////////////////////////////////
// Merge *_coded channels

Channel #(NPCcode + NPCdata) PC_out_packed();
assign {PC_out.code, PC_out.payload} = PC_out_packed.d;
assign PC_out.v = PC_out_packed.v;
assign PC_out_packed.a = PC_out.a;

ChannelMerge coded_merge(PC_out_packed, SF_coded, HB_coded, clk, reset);

endmodule

//////////////////////////////////////////////
// TESTBENCH
module FPGASerializer_tb;

parameter NPCcode = 8;
parameter NPCdata = 24;
parameter Ntime = 48;
parameter N_SF_filts = 10;
parameter N_SF_state = 27;

// our only output goes to the PC
SerializedPCWordChannel PC_out();

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

// output sink
Channel #(NPCcode + NPCdata) PC_out_packed();
assign PC_out_packed.v = PC_out.v;
assign PC_out_packed.d = {PC_out.code, PC_out.payload};
assign PC_out.a = PC_out_packed.a;
ChannelSink #(.ClkDelaysMin(0), .ClkDelaysMax(2)) PC_out_sink(PC_out_packed, clk, reset);

FPGASerializer dut(.*);

endmodule
