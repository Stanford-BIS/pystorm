`include "../src/PCPacker.sv"
`include "ChannelSrcSink.sv"

// PCPacker collects the two upwards streams,
// the BD traffic, and the SpikeGeneratorArray traffic,
// and inserts heartbeat events signalsed by the TimeMgr

module PCPacker #(parameter NPCcode = 8, parameter NPCdata = 24) (
  Channel PC_out,

  // BDSerializer inputs
  SerializedPCWordChannel BD_in,

  // FPGASerializer inputs
  SerializedPCWordChannel FPGA_in,

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
Channel #(NPCcode + NPCdata) BD_packed();
Channel #(NPCcode + NPCdata) FPGA_packed();

assign BD_packed.d = {BD_in.code, BD_in.payload};
assign BD_packed.v = BD_in.v;
assign BD_in.a = BD_packed.a;

assign FPGA_packed.d = {FPGA_in.code, FPGA_in.payload};
assign FPGA_packed.v = FPGA_in.v;
assign FPGA_in.a = FPGA_packed.a;

ChannelMerge packed_merge_root(PC_out, BD_packed, FPGA_packed, clk, reset);

endmodule

