//------------------------------------------------------------------------
// pipe_in_check.v
//
// Received data and checks against pseudorandom sequence for Pipe In.
//
// Even though we're able to produce data in sequence every clock cycle,
// a virtual FIFO is used to provide a way to throttle transfers on a 
// block basis.
//
// Copyright (c) 2005-2010  Opal Kelly Incorporated
// $Rev: 6 $ $Date: 2014-05-02 08:45:29 -0700 (Fri, 02 May 2014) $
//------------------------------------------------------------------------
`timescale 1ns / 1ps
`default_nettype none

module pipe_in_check(
	input  wire            clk,
	input  wire            reset,
	input  wire            pipe_in_write,
	input  wire [31:0]     pipe_in_data,
	output reg             pipe_in_ready,
	input  wire            throttle_set,
	input  wire [31:0]     throttle_val,
	input  wire [2:0]      pattern,
	output reg  [31:0]     error_count
	);

reg  [63:0]  lfsr;
reg  [31:0]  throttle;
reg  [16:0]  level;
wire [31:0]  pg_dout;


pattern_gen #(
		.WIDTH    (32)
	) pg0 (
		.clk      (clk),
		.reset    (reset),
		.enable   (pipe_in_write),
		.mode     (pattern),
		.dout     (pg_dout)
	);


always @(posedge clk) begin
	if (reset == 1'b1) begin
		error_count <= 0;
		throttle    <= throttle_val;
		level       <= 16'd0;
	end else begin
		if (level < 16'd64512) begin
			pipe_in_ready <= 1'b1;
		end else begin
			pipe_in_ready <= 1'b0;
		end
	
		// Update our virtual FIFO level.
		case ({pipe_in_write, throttle[0]})
			2'b00: begin
			end
			
			// Read : Decrease the FIFO level
			2'b01: begin
				if (level > 16'd0) begin
					level <= level - 1'b1;
				end
			end
			
			// Write : Increase the FIFO level
			2'b10: begin
				if (level < 16'd65535) begin
					level <= level + 1'b1;
				end
			end
			
			// Read/Write : No net change
			2'b11: begin
			end
		endcase
	
		// The throttle is a circular register.
		// 1 enabled read or write this cycle.
		// 0 disables read or write this cycle.
		// So a single bit (0x00000001) would lead to 1/32 data rate.
		// Similarly 0xAAAAAAAA would lead to 1/2 data rate.
		if (throttle_set == 1'b1) begin
			throttle <= throttle_val;
		end else begin
			throttle <= {throttle[0], throttle[31:1]};
		end
		
		
		// Check incoming data for validity
		if (pipe_in_write == 1'b1) begin
			if (pipe_in_data[31:0] != pg_dout[31:0]) begin
				error_count <= error_count + 1'b1;
			end
		end
	end
end

endmodule
