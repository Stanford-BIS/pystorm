`define SIMULATION
`include "../src/router/BZ_deserializer.sv"
`include "ChannelSrcSink.sv"

`timescale 10ps/1ps

module BZ_deserializer_tb;

Channel #(32) PC_out_channel();
reg isempty;
reg [10:0] data_in;
reg rdreq;
reg [10:0] data;
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

//ack the cycle after v goes high
always @(posedge clk) begin
	PC_out_channel.a <= PC_out_channel.v;
end

routerDCFIFO input_fifo (
	.data	(data),
	.rdclk	(clk),
	.rdreq	(rdreq),
	.wrclk	(clk),
	.wrreq	(wrreq),
	.q			(data_in),
	.rdempty	(isempty),
	.rdusedw	(),
	.wrfull	(),
	.wrusedw	()
	);

initial begin
	data_in = 0; //wait a bit
	wrreq = 0;
	#25
	wrreq = 1; //write into fifo
	data = 11'b01011011100;
	#70
	data = 11'b01011011101;
	#80
	wrreq = 0;
	// #85
	// data_in = 11'b01011011100; //test wormholes
	// #100
	// data_in = 11'b01110110101; //test non wormholes (we can just put the tail bit high on all of them
	// #60
	// isempty = 1; //test empty idling
	// data_in = 11'b01111111111;
	// #20
	// isempty = 0;
	// data_in = 11'b01111001111;
	// #30
	// isempty = 1; //test empty idling
	// #60
	// isempty = 0;
end

BZ_deserializer #(8, 24, 10) doot(.*); //thank mr skeletal

endmodule // BZ_deserializer_tb