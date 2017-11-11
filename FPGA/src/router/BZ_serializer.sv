`include "../lib/Interfaces.svh"
`include "../lib/Channel.svh"
`include "../lib/ChannelUtil.svh"

//module that converts raw data from the Core to router packets

//////////////////////////////////////////////////////////////
// Core Output Format:
//
//	   10      8      24
//	[ Route | Code | Data ]
//
// We only need to route the least significant 30 bits :)
//		(this makes routing nicer and more efficient)
//////////////////////////////////////////////////////////////
// Router Packet Formats
//	
// Header Packet:
//
//	    10		1
//  [ Route | Tail ]
//
// Data Packet:
//
//	   10	   1
//  [ Data | Tail ]
//
//////////////////////////////////////////////////////////////


module BZ_serializer #(parameter NPCcode = 8, parameter NPCdata = 24, parameter NPCroute = 10)(
	Channel PC_in_channel, //channel from the PC that has data for us
	input is_full, //full signal for the fifo this places stuff into
	output reg [10:0] data_out, //data to write to fifo
	output reg wrreq, //fifo write request
	input clk, reset);

	//store most recently sent route
	//if we want to send another packet to this route, send it on the same worm
	reg [10:0] last_header;

	//construct header packet: contains header and a tail of 0
	reg [10:0] header_packet = {PC_in_channel.d[NPCroute + NPCdata + NPCcode - 1 : NPCcode + NPCdata], 1'b0};


endmodule // BZ_serializer