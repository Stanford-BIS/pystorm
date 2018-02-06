`include "../lib/Interfaces.svh"
`include "../lib/Channel.svh"
`include "../lib/ChannelUtil.svh"

// extracts global route bits from global tag stream

module GlobalTagParser #(
  parameter NPCcode = 7,
  parameter NPCdata = 20,
  parameter NPCroute = 5) (

  // break in simulation if these conditions aren't met
  assert(NPCdata == 20);
  assert(NPCcode == 7);

  GlobalTagCtChannel global_tag_in, //channel that contains tags and global tags
  SerializedPCWordChannelwithRoute ser_out, //channel that contains PC words & a route
  input clk, reset);

  //output code is the same as RI, main tag input
  //a bit hacky but it works
  assign ser_out.code = 30;

	//convert global tag into route
  //using LSBs of global route field
	assign ser_out.route = global_tag_in.global_tag[NPCroute-1:0];

	//get local tag and cts, pad with 0's, put into a payload
	assign ser_out.payload = {global_tag_in.tag, global_tag_in.ct};
 
	//pass-through a and v
	assign ser_out.v = global_tag_in.v;
	assign global_tag_in.a = ser_out.a;

endmodule
