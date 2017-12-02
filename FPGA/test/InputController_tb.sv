`define SIMULATION
`include "../src/Router/InputController.sv"

`timescale 10ps/1ps

module InputController_tb();
reg clk;
reg [10:0] data_in;
reg ready_0;
reg ready_1;
reg almost_empty;
wire read;
wire req_0;
wire req_1;
wire [10:0] data_out;

InputController DUT	(
	.clk				(clk),
	.almost_empty	(almost_empty),
	.ready_0			(ready_0),
	.ready_1			(ready_1),
	.data_in			(data_in),
	.req_0			(req_0),
	.req_1			(req_1),
	.read				(read),
	.data_out		(data_out)
);

initial
begin
	#0
	clk = 0;
	almost_empty=1;
	ready_0=0;
	ready_1=0;
	data_in=11'b01111111101;
	
	#210
	almost_empty=0;
	
	#500
	ready_0=1;
	
	#500
	almost_empty=1;
	
	#200
	almost_empty=0;
	
	#200
	data_in[10]=1;
	
	
	#100
	data_in[10]=0;
	ready_0=0;
	
	#100
	ready_1=1;
	
	#300 
	almost_empty=1;
	
	#100
	almost_empty=0;
	
	#400
	data_in[10]=1;
	
	#200
	ready_1=0;
	
	
	end
	
	
always 
	#50 clk = !clk;
	
always @ (posedge clk)
	if (read)
		data_in[9:0]=data_in[9:0]+1;

endmodule
