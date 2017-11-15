`define SIMULATION
`include "../src/router/BZ_deserializer.sv"
`include "ChannelSrcSink.sv"

module BZ_deserializer_tb;

Channel #(32) PC_out_channel();
reg isempty;
reg [10:0] data_in;
reg rdreq;

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

//ack the cycle after v goes high
always @(posedge clk) begin
	PC_out_channel.a <= PC_out_channel.v;
end

initial begin
	isempty = 0;
	data_in = 11'b01011011100; //test wormholes
	#105
	data_in = 11'b01110110101; //test non wormholes (we can just put the tail bit high on all of them
end

BZ_deserializer #(8, 24, 10) doot(.*); //thank mr skeletal

endmodule // BZ_deserializer_tb