//BrainDrizzle Top

module BrainDrizzle(
input clk, 
input top_clk,
input bottom_clk,
input BD_clk,
input valid_top,
input valid_bottom,
input valid_BD,
input [13:0] top_in,
input [13:0] bottom_in,
input [13:0] BD_in,
output [13:0] top_out,
output [13:0] bottom_out,
output [13:0] BD_out,
output ready_top,
output ready_bottom,
output ready_BD);

wire top_out_FIFO_wr;
wire top_out_FIFO_full;
wire bottom_out_FIFO_wr;
wire bottom_out_FIFO_full;
wire BD_out_FIFO_wr;
wire BD_out_FIFO_full;
wire sel_top;
wire sel_bottom;
wire sel_BD;
wire req_0_top;
wire req_1_top;
wire req_0_bottom;
wire req_1_bottom;
wire req_0_BD;
wire req_1_BD;
wire tail_bottom;
wire tail_BD;

//module allocator (
//clk,
//req_0,
////input req_1,
////input out_FIFO_full,
////input [10:0] data_in_0;
////input [10:0] data_in_1;
////output reg ready_0,
////output reg ready_1,
////output reg out_FIFO_wr,
////output reg [10:0] data_out
////);

//module InputController (
//clk,
////input almost_empty,
////input ready_0,
////input ready_1,
////input [10:0] data_in,
////output reg req_0,
////output reg req_1,
////output  read,
////output reg [10:0] data_out
//);


endmodule

