`include "OKCoreBD.sv" // should have everything we need

module OKBD (
  // OK ifc
	input  wire [4:0]   okUH,
	output wire [2:0]   okHU,
	inout  wire [31:0]  okUHU,
	inout  wire         okAA,

  input wire          sys_clk_p,
  input wire          sys_clk_n,
  input wire          user_reset,
	output wire [3:0]   led,

  // BD ifc
  output logic        BD_out_clk,
  input               BD_out_ready,
  output logic        BD_out_valid,
  output logic [20:0] BD_out_data,

  output logic        BD_in_clk,
  output logic        BD_in_ready,
  input               _BD_in_valid,
  input [33:0]        BD_in_data,
  
  output logic        pReset,
  output logic        sReset,

  input               adc0,
  input               adc1
	);


logic BD_in_valid;
assign BD_in_valid = ~_BD_in_valid;

localparam NPCcode = 8;
localparam NPCdata = 24;
localparam NPCinout = NPCcode + NPCdata;
localparam logic [NPCcode-1:0] NOPcode = 64; // upstream nop code

localparam NBDin = 21;
localparam NBDout = 34;

// internal clocks
wire okClk; // OKHost has a PLL inside it, generates 100MHz clock for the rest of the design
wire sys_clk; // to PLL input
wire BD_in_clk_int; // clocks to BD's handshakers
wire BD_out_clk_int;

// channels between OK ifc and core design
Channel #(NPCinout) PC_downstream();
Channel #(NPCinout) PC_upstream();

// channels between core design and BD ifc
Channel #(NBDin) BD_downstream();
Channel #(NBDout) BD_upstream();

// led control. Flashes for handshakes.
logic [3:0] led_in;
assign led_in[0] = PC_downstream.v;
assign led_in[1] = BD_out_valid;
assign led_in[3] = BD_in_valid;
assign led_in[2] = PC_upstream.v;

// Opal-Kelly HDL host and endpoints, with FIFOs
OKIfc #(
  NPCcode, 
  NOPcode) 
ok_ifc(
	.okUH(okUH),
	.okHU(okHU),
	.okUHU(okUHU),
	.okAA(okAA),
	.led(led),
	.led_in(led_in),
  .okClk(okClk),
  .user_reset(user_reset),
  .PC_downstream(PC_downstream),
  .PC_upstream(PC_upstream));

// get single-ended clock
SysClkBuf sys_clk_buf(.datain(sys_clk_p), .datain_b(sys_clk_n), .dataout(sys_clk));

logic pll_locked;
logic reset_BDIO;
assign reset_BDIO = user_reset;

/////////////////////////////////
// OK -> BD logic
// we implement a nop, register for resets, register to hold data, othewise forward traffic
// msb is nop bit

logic nop_bit;
logic data_bit;
logic pReset_reg_bit;
logic sReset_reg_bit;
logic hold_reg_on_bit;
logic hold_reg_off_bit;

localparam Nctrl = 6;
logic [Nctrl-1:0] ctrl_bits;
assign {nop_bit, data_bit, pReset_reg_bit, sReset_reg_bit, hold_reg_on_bit, hold_reg_off_bit} = ctrl_bits;

logic [NPCinout - NBDin - Nctrl-1:0] unused_bits;
logic [NBDin-1:0] BD_data_bits_down;
assign {ctrl_bits, unused_bits, BD_data_bits_down} = PC_downstream.d;

// registers for resets and hold data value
logic next_pReset, next_sReset;
logic hold_reg_on, next_hold_reg_on;
logic [NBDin-1:0] hold_reg_val;
logic [NBDin-1:0] next_hold_reg_val;

// register updates
always_ff @(posedge okClk, posedge user_reset)
  if (user_reset == 1) begin
    hold_reg_on <= 0;
    sReset <= 1;
    pReset <= 1;
    hold_reg_val <= 0;
  end
  else begin
    hold_reg_on <= next_hold_reg_on;
    sReset <= next_sReset;
    pReset <= next_pReset;
    hold_reg_val <= next_hold_reg_val;
  end

always_comb begin
  next_pReset = pReset;
  next_sReset = sReset;
  next_hold_reg_on = hold_reg_on;
  next_hold_reg_val = hold_reg_val;
  if (PC_downstream.v == 1) begin
    if (pReset_reg_bit == 1) begin
      next_pReset = PC_downstream.d[0];
    end
    else if (sReset_reg_bit == 1) begin
      next_sReset = PC_downstream.d[0];
    end
    else if (hold_reg_on_bit == 1) begin
      next_hold_reg_on = 1;
      next_hold_reg_val = PC_downstream.d[NBDin-1:0];
    end
    else if (hold_reg_off_bit == 1) begin
      next_hold_reg_on = 0;
    end
  end
end

// pass down data for data_bit == 1
assign BD_downstream.d = data_bit == 1 ? PC_downstream.d[NBDin-1:0] : 'X;
assign BD_downstream.v = PC_downstream.v & data_bit;

// auto-ack for non-data
assign PC_downstream.a = data_bit & BD_downstream.a | nop_bit | pReset_reg_bit | sReset_reg_bit | hold_reg_on_bit | hold_reg_off_bit;

// can still try to send data with data held (you need to, to actually send the held word)
// data lines will stay at held value, but handshake will occur for each downstream word
// this implements the data hold
logic [NBDin-1:0] BD_out_data_from_HSer;
assign BD_out_data = hold_reg_on == 1 ? hold_reg_val : BD_out_data_from_HSer;


/////////////////////////////////
// BD -> OK logic
// just a serializer
Serializer #(.Nin(NBDout), .Nout(NPCinout)) serializer(PC_upstream, BD_upstream, okClk, user_reset);


// PLL generates BD_IO_clk
BDClkGen bd_clk_gen(
  .BD_in_clk_int(BD_in_clk_int),
  .BD_in_clk_ext(BD_in_clk),
  .BD_out_clk_int(BD_out_clk_int),
  .BD_out_clk_ext(BD_out_clk),
  .pll_locked(pll_locked),
  .clk(sys_clk), 
  .reset(user_reset));

// BD handshakers and FIFOs
BDIfc BD_ifc(
  .core_up(BD_upstream),
  .core_dn(BD_downstream),
  .BD_out_ready(BD_out_ready),
  .BD_out_valid(BD_out_valid),
  .BD_out_data(BD_out_data_from_HSer),
  .BD_in_ready(BD_in_ready),
  .BD_in_valid(BD_in_valid),
  .BD_in_data(BD_in_data),
  .core_clk(okClk),
  .BD_in_clk_int(BD_in_clk_int),
  .BD_out_clk_int(BD_out_clk_int),
  .reset(reset_BDIO));

endmodule
