module OKIfc #(
  parameter NPCcode = 8,
  parameter logic [NPCcode-1:0] NOPcode = 64) // upstream nop code
(
	input  wire [4:0]   okUH,
	output wire [2:0]   okHU,
	inout  wire [31:0]  okUHU,
	inout  wire         okAA,
  output wire okClk,
  Channel data_out,
  Channel data_in);

localparam NPCdata = 24;
localparam NPCin = NPCcode + NPCdata;
localparam Nfifo = 9; // 512-word FIFO
localparam Nblock = 7; // 128-word block size for BTPipe

// OK internal bus
wire [112:0] okHE;
wire [64:0]  okEH;

// okHost, connects external pins to bus
okHost OK_host(
	.okUH(okUH),
	.okHU(okHU),
	.okUHU(okUHU),
	.okAA(okAA),
	.okClk(okClk),
	.okHE(okHE), 
	.okEH(okEH)
);

// okWireOR, needed when you have more than one module using okEH
// in this case, we have 2
localparam NOKmodules = 2;
wire [65*NOKmodules-1:0] okEHx;
okWireOR # (.N(NOKmodules)) wireOR (okEH, okEHx);

// pipe in is addr 0
// pipe out is addr 1

////////////////////////////////////////////
// okBTPipeIn/FIFO_in

wire        pipe_in_write;
wire        pipe_in_ready;
wire [31:0] pipe_in_data;

logic [Nfifo-1:0] FIFO_in_count;
logic [NPCin-1:0] FIFO_in_data_out;
logic FIFO_in_rd_ack;
logic FIFO_in_empty;

okBTPipeIn OK_BT_pipe_in(
  .okHE(okHE),
  .okEH(okEHx[0*65+:65]),
  .ep_addr(8'h80),
  .ep_write(pipe_in_write),
  .ep_blockstrobe(),
  .ep_dataout(pipe_in_data),
  .ep_ready(pipe_in_ready));

// input FIFO, compute ready signal using usedw
// if the MSBs aren't all one, there's 128 words free
//assign pipe_in_ready = ~(&FIFO_in_count[Nfifo-1:Nblock]);

//PipeInFIFO fifo_in(
//  .data(pipe_in_data),
//  .wrreq(pipe_in_write),
//  .q(FIFO_in_data_out),
//  .rdreq(FIFO_in_rd_ack),
//  .empty(FIFO_in_empty),
//  .usedw(FIFO_in_count),
//  .clock(okClk));
//
//// handshake output
//assign data_out.v = ~FIFO_in_empty;
//assign data_out.d = FIFO_in_data_out;
//assign FIFO_in_rd_ack = data_out.a;

////////////////////////////////////////////
// okPipeOut/FIFO_out
// this is a standard PipeOut
// we must be prepared to feed it every clock cycle
// when there is no data, we feed it a NOP

wire        pipe_out_read;
wire [31:0] pipe_out_data;

okPipeOut OK_pipe_out(
  .okHE(okHE),
  .okEH(okEHx[1*65+:65]),
  .ep_addr(8'ha0),
  .ep_datain(pipe_out_data),
  .ep_read(pipe_out_read));

//always_comb
//  if (pipe_out_read == 1) // pipe needs data this cycle!
//    if (data_in.v == 1) begin // use data from channel
//      pipe_out_data = data_in.d;
//      data_in.a = 1;
//    end
//    else begin
//      pipe_out_data = {NOPcode, {NPCdata{1'b0}}};
//      data_in.a = 0;
//    end
//  else begin
//    pipe_out_data = 'X;
//    data_in.a = 0;
//  end

//// unfinished block-triggered version with FIFO
//wire        pipe_out_read;
//wire        pipe_out_ready;
//wire [31:0] pipe_out_data;
//
//logic [NPCin-1:0] FIFO_out_data_in;
//logic FIFO_out_wr_en;
//logic FIFO_out_full;
//
//okBTPipeOut OK_BT_pipe_out(.okHE(okHE),
//  .okEH(okEH)
//  .ep_addr(8'h01),
//  .ep_read(pipe_out_read),
//  .ep_blockstrobe(),
//  .ep_datain(pipe_out_data),
//  .ep_ready(pipe_out_ready));
//
//// output FIFO, compute ready signal using usedw
//// FIFO must have at least 128 entries
////assign pipe_out_ready = FIFO_out_count[Nblock];
//
//PipeOutFIFO fifo_out(
//  .data(FIFO_out_data_in),
//  .wrreq(FIFO_out_wr_en),
//  .q(pipe_out_data),
//  .rdreq(pipe_out_read),
//  .full(FIFO_out_full),
//  .clock(okClk));

endmodule
