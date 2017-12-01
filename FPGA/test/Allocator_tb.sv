`define SIMULATION
`include "../src/router/allocator.sv"
`timescale 10ps/1ps

module Allocator_tb();
reg clk;
reg req_0;
reg req_1;
reg out_FIFO_full;
reg [10:0] data_in_0;
reg [10:0] data_in_1;
wire ready_0;
wire ready_1;
wire out_FIFO_wr;
wire [10:0] data_out;



//DUT
allocator DUT (
	.clk				(clk),
	.req_0			(req_0),
	.req_1			(req_1),
	.out_FIFO_full	(out_FIFO_full),
	.data_in_0		(data_in_0),
	.data_in_1		(data_in_1),
	.ready_0			(ready_0),
	.ready_1			(ready_1),
	.out_FIFO_wr	(out_FIFO_wr),
	.data_out		(data_out)
);

initial
begin
	#0
	clk = 0;
	req_0=0;
	req_1 = 0;
	out_FIFO_full = 0;
	data_in_0 = 0;
	data_in_1 = 0;
	
	#210
	req_0 = 1;
	
	#300
	req_0 = 0;
	
	#100
	data_in_0[10]=1;
	
	#100
	data_in_0[10]=0;
	req_1=1;
	
	#300	  
	req_1=0;
	
	#300
	req_1=1;
	
	#300
	req_1=0;
	
	#400
	data_in_1[10]=1;
	
	#100
	data_in_1[10]=0;
	
	#100
	req_1=1;
	req_0=1;
	
	#300
	req_0=0;
	
	#100
	data_in_0[10]=1;
	
	#100
	data_in_0[10]=0;
	
	#200
	req_1=0;
	
	#100
	out_FIFO_full=1;
	
	#500
	out_FIFO_full=0;
	
	#100
	data_in_1[10]=1;
	
	//#600
	
	//$finish;
	
end
  
always @ (negedge clk)
begin
	if (ready_0)
	begin
		data_in_0 ={data_in_0[10], data_in_0[9:0] + 1};
	end
	if (ready_1)
	begin
		data_in_1 ={data_in_1[10], data_in_1[9:0] + 1};
	end
end



always 
	#50 clk = !clk;

endmodule