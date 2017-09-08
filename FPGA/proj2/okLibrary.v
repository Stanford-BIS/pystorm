//------------------------------------------------------------------------
// okLibrary.v
//
// FrontPanel Library Module Declarations (Verilog)
// ZEM5305
//
// Copyright (c) 2004-2011 Opal Kelly Incorporated
// $Rev: 980 $ $Date: 2011-08-19 14:17:52 -0500 (Fri, 19 Aug 2011) $
//------------------------------------------------------------------------

module okHost(
	input  wire [4:0]   okUH,
	output reg  [2:0]   okHU,
	inout  wire [31:0]  okUHU,
	inout  wire         okAA,
	output wire         okClk,
	output wire [112:0] okHE,
	input  wire [64:0]  okEH
);
	
	wire [38:0] okHC;
	wire [37:0] okCH;
	
	wire       pll_locked;
	
	
	assign okHC[38]   = ~pll_locked;
	assign okHC[37]   = okAA;
	assign okHC[36:5] = data_in;
	assign okHC[4:1]  = ctrl_in;
	assign okClk      =  okHC[0];
	
	
	// Clock
	ok_altera_pll #(
		.phase ( "-2014 ps" )
	) ok_altera_pll0 (
		.refclk ( okUH[0] ),
		.rst ( ),
		.outclk_0 ( okHC[0] ),
		.locked ( pll_locked )
		);
	
	//------------------------------------------------------------------------
	// Bidirectional IOB registers
	//------------------------------------------------------------------------
	reg        data_valid;
	reg [31:0] data_out;
	reg [31:0] data_in;
	reg [4:1]  ctrl_in;
	
	//Tristate
	assign okUHU = (data_valid) ? 32'bZ : data_out;
	
	// Input Registering
	always @ (posedge okHC[0]) begin
		data_in <= okUHU;
	end
	
	// Output Registering
	always @ (posedge okHC[0]) begin
		data_out <= okCH[34:3];
	end
	
	// Tristate Drive
	always @ (posedge okHC[0]) begin
		data_valid <= ~okCH[36];
	end
	
	assign okAA = (okCH[37]) ? 1'bZ : okCH[35];
	
	
	//------------------------------------------------------------------------
	// Output IOB registers
	//------------------------------------------------------------------------
	always @ (posedge okHC[0]) begin
		okHU[2] <= okCH[2];
		okHU[0] <= okCH[0];
		okHU[1] <= okCH[1];
	end
	
	
	//------------------------------------------------------------------------
	// Input IOB registers
	//  - First registered on DCM0 (positive edge)
	//  - Then registered on DCM0 (negative edge)
	//------------------------------------------------------------------------
	always @ (posedge okHC[0]) begin
		ctrl_in[1]   <= okUH[1];
		ctrl_in[4:2] <= okUH[4:2];
	end
	
	okCoreHarness core0(.okHC(okHC), .okCH(okCH), .okHE(okHE), .okEH(okEH));
	
endmodule

module okWireOR # (parameter N = 1)	(
	output reg  [64:0]     okEH,
	input  wire [N*65-1:0] okEHx
	);

	integer i;
	always @(okEHx)
	begin
		okEH = 0;
		for (i=0; i<N; i=i+1) begin: wireOR
			okEH = okEH | okEHx[ i*65 +: 65 ];
		end
	end
endmodule
