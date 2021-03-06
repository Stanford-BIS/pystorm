`define SIMULATION
`include "../src/Router/InputController.sv"

`timescale 10ps/1ps

module InputController_tb();
reg clk;
reg [10:0] data_in;
reg ready_0;
reg ready_1;
reg empty;
reg reset;
wire read;
wire req_0;
wire req_1;
wire [10:0] data_out;


InputController DUT	(
	.clk				(clk),
	.reset			(reset),
	.empty			(empty),
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
	empty=1;
	ready_0=0;
	ready_1=0;
	data_in=11'b01111111101;
	reset=1;
	
	#210
	reset=0;
	
	#200
	empty=0;
	
	#500
	ready_0=1;
	
	#500
	empty=1;
	
	#200
	empty=0;
	
	#500
	ready_0=0;
	
	#300
	ready_0=1;
	
	#200
	data_in[10]=1;
	
	
	#100
	data_in[10]=0;
	
	
	#500
	ready_0=0;
	
	#100
	ready_1=1;
	
	#300 
	empty=1;
	
	#100
	empty=0;
	
	#400
	data_in[10]=1;
	#100
	empty=1;
	ready_1=0;
	
	
	end
	
	
always 
	#50 clk = !clk;
	
always @ (posedge clk)
	if (read && ~empty)
		data_in[9:0]=data_in[9:0]+1;

endmodule
