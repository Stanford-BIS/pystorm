//-------------------------------------------------------------------------
// pattern_gen.v
//
// Parameterizable pattern generator for bus testing.
//
// DATA_PATTERN:
//    000 - Counter, starting at 0.  32-bits wide.
//    001 - LFSR, 32-bit: x^32 + x^22 + x^2 + 1
//    010 - Walking 1's
//    011 - Walking 0's
//    100 - Hammer (alternating 1's 0's on the data bus)
//    101 - Neighbor (hammer, but with a single 0 at all times)
//    110 - N/A
//    111 - N/A
//
// Copyright (c) 2010-2011  Opal Kelly Incorporated
// CONFIDENTIAL AND PROPRIETARY
// $Rev: 6 $ $Date: 2014-05-02 08:45:29 -0700 (Fri, 02 May 2014) $
//-------------------------------------------------------------------------

`timescale 1ps/1ps

module pattern_gen (
	input  wire                  clk,
	input  wire                  reset,
	input  wire                  enable,
	input  wire [2:0]            mode,
	output wire [WIDTH-1:0]      dout
	);


parameter  WIDTH      = 32;
parameter  LFSR_RESET = 32'h04030201;


reg  [WIDTH-1:0]  neighbor;
reg  [2:0]        mode_d;
reg               toggle;
reg  [63:0]       preg;

assign dout = preg[WIDTH-1:0];


// Resets when RESET=1.
// Generates a new word in the pattern when ENABLE=1.
always @(posedge clk) begin
	if (reset == 1'b1) begin
		toggle   <= 1'b1;
		mode_d   <= mode;
		neighbor <= 32'hfffffffe;
		case (mode)
			3'b000: preg <= 32'h00000001;            // Counter
			3'b001: preg <= LFSR_RESET;              // LFSR
			3'b010: preg <= 32'h00000001;            // Walking 1's
			3'b011: preg <= 32'hfffffffe;            // Walking 0's
			3'b100: preg <= 32'h00000000;            // Hammer
			3'b101: preg <= 32'h00000000;            // Neighbor (hammer except one bit is always low)
			3'b110: preg <= 32'h00000000;
			3'b111: preg <= 32'h00000000;
		endcase
	end else begin
		if (enable == 1'b1) begin
			toggle      <= ~toggle;
			neighbor    <= toggle ? neighbor : {neighbor[WIDTH-2:0], neighbor[WIDTH-1]};
			case (mode_d)
				3'b000: preg <= preg + 1'b1;                                   // Counter
				3'b001: preg <= {preg[30: 0], preg[31] ^ preg[21] ^ preg[1]};  // LFSR
				3'b010: preg <= {preg[WIDTH-2:0], preg[WIDTH-1]};              // Walking 1's
				3'b011: preg <= {preg[WIDTH-2:0], preg[WIDTH-1]};              // Walking 0's
				3'b100: preg <= {WIDTH{toggle}};                               // Hammer
				3'b101: preg <= toggle ? neighbor : 1'h0;                      // Neighbor
				3'b110: preg <= 32'h00000000;
				3'b111: preg <= 32'h00000000;
			endcase
		end
	end
end


endmodule
