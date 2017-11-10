`include "../lib/Interfaces.svh"
`include "../lib/Channel.svh"
`include "../lib/ChannelUtil.svh"
//simple interfact between global tag ct channel and serialized pc channel

module GlobalTagParser #(
  parameter NBDdata_in = 34,
  parameter Nglobal = 14,
  parameter Ntag = 11,
  parameter Nct = 9,

  parameter Ncode = 8,
  parameter Ndata_out = 24, 
  parameter Nroute = 11)(

  GlobalTagCtChannel global_tag_in, //channel that contains tags and global tags
  SerializedPCWordChannelwithRoute ser_out, //channel that contains PC words & a route
  input clk, reset);

  	//output code is the same as RI, main tag input
  	//a bit hacky but it works
  	assign ser_out.code = 30;

	//convert global tag into route
	assign ser_out.route = global_tag_in.global_tag[Nroute-1:0];

	//get local tag and cts, pad with 0's, put into a payload
	assign ser_out.payload = {4'b0, global_tag_in.tag, global_tag_in.ct};
 
	//pass-through a and v
	assign ser_out.v = global_tag_in.v;
	assign global_tag_in.a = ser_out.a;

endmodule