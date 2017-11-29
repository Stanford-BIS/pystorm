`include "../lib/Interfaces.svh"
`include "../lib/Channel.svh"
`include "../lib/ChannelUtil.svh"

// PCPacker collects the two upwards streams,
// the BD traffic, and the SpikeGeneratorArray traffic,
// and inserts heartbeat events signalsed by the TimeMgr

module PCPacker #(parameter NPCcode = 8, parameter NPCdata = 24, parameter NPCroute = 10, parameter GO_HOME_rt = -512) (
  Channel PC_out,

  // BDSerializer inputs
  SerializedPCWordChannel BD_in,

  // FPGASerializer inputs
  SerializedPCWordChannel FPGA_in,

  // GlobalTagParser inputs
  SerializedPCWordChannelwithRoute Global_in,

  input clk, reset);

//////////////////////////////////////////////
// FPGA packet formats
//
// All words look like this:
//
//    8      24 
// [ code | data ]
//
// FPGA and BD codes share the same 4-bit space,
// there's no FPGA/BD bit
//
// Serialized data is always transmitted LSBs first
//
//////////////////////////////////////////////
// BD words
//
//  MSB            LSB
//    8            24 
// [ code | BD_payload_chunk ]
//
// BD words use codes 0-12 (13 funnel leaves)
//
//////////////////////////////////////////////
// FPGA-filtered spike/tag stream
//
// 2 words per event:
//
//  MSB                  LSB
//      8             
// [ code=13 | state[23:0] ]
// 
//      8         21            3
// [ code=13 | filt_idx | state[26:24]  ]
//
// (note: 27 state bits is unlikely to change, it's the DSP width
//  21 is just the bits remaining for filt_idx, there may not be 2**21 filters)
//
//////////////////////////////////////////////
// FPGA-generated heartbeat
// (split into two packets)
//
//  MSB                      LSB
//      8             24
// [ code=14 |  time_bits[23:0] ]
//
//      8             24
// [ code=14 |  time_bits[47:24] ]

// pack bits, then merge
Channel #(NPCcode + NPCdata + NPCroute) BD_packed();
Channel #(NPCcode + NPCdata + NPCroute) FPGA_packed();
Channel #(NPCcode + NPCdata + NPCroute) Global_packed();

//pack BD, add go home route
assign BD_packed.d = {GO_HOME_rt, BD_in.code, BD_in.payload};
assign BD_packed.v = BD_in.v;
assign BD_in.a = BD_packed.a;

//pack FPGA, add go home route
assign FPGA_packed.d = {GO_HOME_rt, FPGA_in.code, FPGA_in.payload};
assign FPGA_packed.v = FPGA_in.v;
assign FPGA_in.a = FPGA_packed.a;

//pack BD-to-BD tag data
assign Global_packed.d = {Global_in.route, Global_in.code, Global_in.payload};
assign Global_packed.v = Global_in.v;
assign Global_in.a = Global_packed.a;

//double merge
Channel #(NPCcode + NPCdata + NPCroute) temp_merge();
ChannelMerge packed_merge_initial(temp_merge, BD_packed, FPGA_packed, clk, reset);
ChannelMerge packed_merge_root(PC_out, temp_merge, Global_packed, clk, reset);

endmodule

