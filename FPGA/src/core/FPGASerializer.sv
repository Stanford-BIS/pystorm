`include "../lib/Channel.svh"
`include "../lib/ChannelUtil.svh"
`include "../lib/Interfaces.svh"
`include "Serializer.sv"

module FPGASerializer #(
  parameter NPCcode = 7,
  parameter NPCdata = 20,
  parameter Ntime_full = 48,
  parameter N_SF_filts = 10,
  parameter N_SF_state = 27) (

  // our only output goes to the PC
  SerializedPCWordChannel PC_out,

  // TimeMgr inputs
  input send_HB_up_pulse, // pulses every send_HB_up_every * time_unit, for 1 clk cycle
  input [Ntime_full-1:0] time_elapsed, // current wall time, can pack into unused spike bits for higher resolution
   
  // Filtered spikes
  SpikeFilterOutputChannel SF_in,

  input clk, reset);

localparam logic [NPCcode-1:0] SF_code = 14;
localparam logic [NPCcode-1:0] HB_lsb_code = 15;
localparam logic [NPCcode-1:0] HB_msb_code = 16;

// the packing and serialization of time is hardcoded 
if (NPCdata != 20)
  $error("illegal value for NPCdata != 20");
if (Ntime_full != 48)
  $error("illegal value for Ntime_full != 48");

localparam Ntime = NPCdata * 2;

// 1. pack/convert inputs into Channels
// 2. serialize, 
// 3. append codes
// 4. merge
// 
// could make this more regular by forming a ChannelArray of pre-packed streams
// doing step 2,3 in a loop
// for now there are only 2 inputs, though

Channel #(Ntime_full) HB_packed_full();
Channel #(Ntime*2) HB_packed();
Channel #(Ntime+2) HB_packed_with_lsb_msb();
Channel #(N_SF_filts + N_SF_state) SF_packed();

Channel #(NPCdata+1) HB_serialized(); // hiding lsb/msb in the extra bit
Channel #(NPCdata) SF_serialized();

Channel #(NPCcode + NPCdata) HB_coded();
Channel #(NPCcode + NPCdata) SF_coded();

//////////////////////////////////////////////
// Heartbeat

// pulse -> channel
PulseToChannel #(Ntime_full) HB_pulse_to_channel(HB_packed_full, time_elapsed, send_HB_up_pulse, clk, reset);

// chop off highest order time bits
assign HB_packed.v = HB_packed_full.v;
assign HB_packed.d = HB_packed_full.d[Ntime-1:0];
assign HB_packed_full.a = HB_packed.a;

// pack in lsb/msb bit
assign HB_packed_with_lsb_msb.v = HB_packed.v;
assign HB_packed_with_lsb_msb.d = {1'b1, HB_packed.d[Ntime-1:Ntime/2], 1'b0, HB_packed.d[Ntime/2-1:0]};
assign HB_packed.a = HB_packed_with_lsb_msb.a;

// serialize
Serializer #(.Nin(Ntime+2), .Nout(NPCdata+1)) HB_serializer(HB_serialized, HB_packed_with_lsb_msb, clk, reset);

// append code
always_comb
  if (HB_serialized.d[NPCdata] == 1) 
    HB_coded.d = {HB_msb_code, HB_serialized.d[NPCdata-1:0]}; // time msb
  else
    HB_coded.d = {HB_lsb_code, HB_serialized.d[NPCdata-1:0]}; // time lsb
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
