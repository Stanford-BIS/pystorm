`include "../lib/Interfaces.svh"
`include "../lib/Channel.svh"
`include "../lib/ChannelUtil.svh"

//module that converts packets to core data

//////////////////////////////////////////////////////////////
// Router Packet Formats
//	
// Header Packet:
//
//	    10		1
//  [ Route | Tail ]
//
//	Header packets will always be zero by the time we get them, so we can ignore them
//
// Data Packet:
//
//	   10	   1
//  [ Data | Tail ]
//
//////////////////////////////////////////////////////////////
// Core Input Format:
//
//     8      24
//	[ Code | Data ]
//
// The first 2 bits of the code are always 0, and are *not routed*
//////////////////////////////////////////////////////////////


module BZ_deserializer #(parameter NPCcode = 8, parameter NPCdata = 24, parameter NPCroute = 10)(
	Channel PC_out_channel, //output channel for the Core
	input isempty, //isempty signal for the fifo feeding us packets
	input [10:0] data_in, //data from the fifo
	output reg rdreq, //read request for fifo
	input clk, reset);

	//reg for tail bit
	reg tail_bit = 1'b0;

	//FSM States: i'll write this up once i fix it
	reg [2:0] state = 3'b0;
	reg [2:0] next_state = 3'b0;
	always_comb
		case(state)
			3'd0, 3'd1, 3'd2: next_state = isempty ? state : (state + 3'd1);
			3'd3: next_state = tail_bit ? 3'd4 : 3'd5;
			3'd4: next_state = PC_out_channel.a ? 3'd0 : state;
			3'd5: next_state = PC_out_channel.a ? 3'd1 : state;
			3'd6: next_state = isempty ? state : 3'd0;
			default: next_state = 3'd4; //default to idling for header
		endcase

	always @(posedge clk) begin
		if(!reset) begin
			state <= next_state; //ff the state
		end
		else begin
			state <= 3'd6; //reset to idling for header (special state)
		end
	end

	reg [31:0] to_transmit;
	assign to_transmit[31:30] = 2'b0; //first 2 bits are always zero
	assign PC_out_channel.d = to_transmit;

	always @(posedge clk) begin //do these synchronously to avoid issues
		case(state)
			3'd1: begin
				if (!isempty) begin
					to_transmit[29:20] <= data_in[10:1];
				end
				else begin
					to_transmit[29:0] <= to_transmit[29:0];
				end
			end

			3'd2: begin
				if (!isempty) begin
					to_transmit[19:10] <= data_in[10:1];
				end
				else begin
					to_transmit[29:0] <= to_transmit[29:0];
				end
			end

			3'd3: begin
				if (!isempty) begin
					to_transmit[9:0] <= data_in[10:1];
				end
				else begin
					to_transmit[29:0] <= to_transmit[29:0];
				end
			end

			default: begin
				to_transmit[29:0] <= to_transmit[29:0];
			end
		endcase
	end

	always_comb
		case(state)
			3'd0: begin
				rdreq = !isempty; 
				PC_out_channel.v = 0;
			end//just ignore the header data

			3'd1: begin
				rdreq = !isempty;
				PC_out_channel.v = 0;
			end

			3'd2: begin
				rdreq = !isempty;
				PC_out_channel.v = 0;
			end

			3'd3: begin
				rdreq = 1;
				PC_out_channel.v = 0;
				tail_bit <= data_in[0]; //get tail bit
			end

			3'd4, 3'd5: begin
				rdreq = 0; 
				PC_out_channel.v = !isempty; //pull valid high once we have a full word
			end

			3'd6: begin
				rdreq = 0;
				PC_out_channel.v = 0;
			end

		endcase
	
endmodule // BZ_deserializer