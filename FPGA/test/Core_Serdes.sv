`define SIMULATION
`include "../src/core/Core.sv"
`include "ChannelSrcSink.sv"
`include "../src/router/BZ_deserializer.sv"
`include "../src/router/BZ_serializer.sv"

module Core_Serdes;

parameter NPCcode = 8;
parameter NPCdata = 24;
parameter NPCroute = 10;

parameter NBDdata_in = 34;
parameter NBDdata_out = 21;

reg pReset;
reg sReset;

reg adc0;
reg adc1;

//des fifo and data
reg isempty;
reg [10:0] data_in;
reg rdreq_des;
reg [10:0] data;
reg wrreq_des;

//ser fifo and data
reg is_full;
reg [10:0] data_out;
reg wrreq_ser;
reg [10:0] main_output;
reg rdreq_ser;

// PC-side
Channel #(NPCcode + NPCdata) PC_in();
Channel #(NPCcode + NPCdata+NPCroute) PC_out();

// BD-side
Channel #(NBDdata_out) BD_out();
Channel #(NBDdata_in) BD_in();

//ser and des
Channel #(NPCcode + NPCdata) Des_out();
Channel #(NPCcode + NPCdata+NPCroute) Ser_in();

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

DCChannelFIFO32 input_channel_fifo(Des_out, PC_in, clk, clk, reset);

routerDCFIFO input_fifo (
	.data	(data),
	.rdclk	(clk),
	.rdreq	(rdreq_des),
	.wrclk	(clk),
	.wrreq	(wrreq_des),
	.q			(data_in),
	.rdempty	(isempty),
	.rdusedw	(),
	.wrfull	(),
	.wrusedw	()
	);

DCChannelFIFO42 output_channel_fifo(PC_out, Ser_in, clk, clk, reset);

routerDCFIFO output_fifo (
	.data	(data_out),
	.rdclk	(clk),
	.rdreq	(rdreq_ser),
	.wrclk	(clk),
	.wrreq	(wrreq_ser),
	.q			(main_output),
	.rdempty	(),
	.rdusedw	(),
	.wrfull	(is_full),
	.wrusedw	()
	);

initial begin
	data_in = 0; //wait a bit
	wrreq_des = 0;
	rdreq_ser = 0;
	#25
	rdreq_ser = 1;
	wrreq_des = 1; //write into fifo
	data = 11'b01011011100;
	#70
	data = 11'b01011011101;
	#80
	wrreq_des = 0;
end

//RandomChannelSrc #(.N(NPCcode + NPCdata), .ClkDelaysMin(0), .ClkDelaysMax(4)) PC_in_src(PC_in, clk, reset);
//ChannelSink #(.ClkDelaysMin(0), .ClkDelaysMax(1)) PC_out_sink(PC_out, clk, reset);

BZ_deserializer #(8, 24, 10) des( Des_out, //Goes to the Core
	isempty, //isempty signal for the fifo feeding us packets
	data_in, //data from the fifo
	rdreq_des, //read request for fifo
	clk, reset); //thank mr skeletal

BZ_serializer #(8, 24, 10) ser( Ser_in, //channel from the Core that has data for us
	is_full, //full signal for the fifo this places stuff into
	data_out, //data to write to fifo
	wrreq_ser, //fifo write request
	clk, reset); //doot doot

RandomChannelSrc #(.N(NBDdata_in)) BD_in_src(BD_in, clk, reset);

ChannelSink BD_out_sink(BD_out, clk, reset);

Core dut(.*);

endmodule
