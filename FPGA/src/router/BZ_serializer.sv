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
	Channel PC_in_channel, //channel from the Core that has data for us
	input is_full, //full signal for the fifo this places stuff into
	output reg [10:0] data_out, //data to write to fifo
	output reg wrreq, //fifo write request
	input clk, reset);

	//current header
	wire [10:0] current_header;
	assign current_header = {1'b0, PC_in_channel.d[NPCroute + NPCdata + NPCcode - 1 : NPCcode + NPCdata]};

	//store header packet
	reg [10:0] header_packet = 11'b0;

	//data, to be sliced
	reg [29:0] data = 30'b0;

	//FSM: states are:
	//0 - idle, waiting for valid
	//1 - write header
	//2 - 4, serialize packets 1-3
	reg [2:0] state = 3'b0;
	reg [2:0] next_state;
	always_comb
		case(state)
			3'd0: next_state = PC_in_channel.v & !is_full ? 3'd1 : 3'd0; //idle; begin serializing when input is valid and FIFO is not full
			3'd1, 3'd2, 3'd3: next_state = is_full ? state : (state + 3'd1); //for transmit states, just move forward
			3'd4: next_state = is_full ? state : (PC_in_channel.v ? ((header_packet == current_header) ? 3'd2 : 3'd1) : 3'd0); //done serializing, check next header if valid
			default: next_state = 3'b0;
		endcase // state

	always @(posedge clk or posedge reset) begin
		if (reset==1) begin
			header_packet <= 11'b0; //assign header packet
			data <= 30'b0; //assign data
		end
		else if(state == 3'd0 | state == 3'd4) begin
			header_packet <= {1'b0, PC_in_channel.d[NPCroute + NPCdata + NPCcode - 1 : NPCcode + NPCdata]}; //assign header packet
			data <= PC_in_channel.d[29:0]; //assign data
		end
		if (reset==1) begin
			state <= 3'b0;
		end
		else begin
			state <= next_state; //ff the state
		end

	end

	always @(*) begin
		case(state)
			3'd0: begin
					wrreq = 1'b1;
					data_out = 11'b0;
					PC_in_channel.a = 1'b0;
				  end

			3'd1: begin
					wrreq = 1'b1;
					data_out = header_packet; //output header packet
					PC_in_channel.a = 1'b0;
					end

			3'd2: begin
					wrreq = 1'b1;
					data_out = {1'b0, data[29:20]}; //data 1
					PC_in_channel.a = !is_full; //ack
					end

			3'd3: begin
					wrreq = 1'b1;
					data_out = {1'b0, data[19:10]}; //data 2
					PC_in_channel.a = 1'b0;
					end

			3'd4: begin
					wrreq = 1'b1;
					if (PC_in_channel.v & (header_packet == current_header)) begin
						data_out = {1'b0, data[9:0]}; //data 3, no tail
					end
					else begin
						data_out = {1'b1, data[9:0]}; //data 3, yes tail
					end
					PC_in_channel.a = 1'b0;
					end

			default: begin
					wrreq = 1'b0;
					data_out = 11'b0;
					PC_in_channel.a = 1'b0;
				  	end
		endcase
	end

endmodule // BZ_serializer