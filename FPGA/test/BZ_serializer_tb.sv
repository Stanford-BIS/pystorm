`define SIMULATION
`include "../src/router/BZ_serializer.sv"
`include "ChannelSrcSink.sv"

module BZ_serializer_tb;

Channel #(42) PC_in_channel();
reg is_full;
reg [10:0] data_out;
reg wrreq;

// clock
logic clk;
parameter Tclk = 10;
always #(Tclk/2) clk = ~clk;
initial clk = 0;

// reset
logic reset;
initial begin
  reset <= 0;
  @(posedge clk) reset <= 1;
  @(posedge clk) reset <= 0;
end

initial begin
	is_full = 0;
	PC_in_channel.v = 1;
	PC_in_channel.d = 42'b0;

	repeat (20) @(posedge clk) begin
		if(PC_in_channel.a == 1) begin
			PC_in_channel.d <= PC_in_channel.d + 1; //check wormholes
		end
	end
	repeat (20) @(posedge clk) begin
		if(PC_in_channel.a == 1) begin
			PC_in_channel.d <= PC_in_channel.d + 40'b10000000000000000000000000000000000; //check non-wormholes
		end
	end
end

BZ_serializer #(8, 24, 10) doot(.*); //doot doot

endmodule // BZ_serializer_tb