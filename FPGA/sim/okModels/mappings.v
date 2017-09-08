//------------------------------------------------------------------------
// mappings.v
//
// Description:
//  This file contains ok1 mappings for simulation.
//
//------------------------------------------------------------------------
// Copyright (c) 2005-2010 Opal Kelly Incorporated
// $Rev$ $Date$
//------------------------------------------------------------------------

// Endpoint input harness
localparam okHE_DATAL         = 47;
localparam okHE_DATAH         = 78;
localparam okHE_ADDRL         = 36;
localparam okHE_ADDRH         = 43;

localparam okHE_CLK           = 3;
localparam okHE_RESET         = 0;
localparam okHE_READ          = 46;
localparam okHE_WRITE         = 1;
localparam okHE_WIREUPDATE    = 45;
localparam okHE_TRIGUPDATE    = 2;
localparam okHE_BLOCKSTROBE   = 44;
localparam okHE_REGADDRL      = 81;
localparam okHE_REGADDRH      = 112;
localparam okHE_REGWRITE      = 80;
localparam okHE_REGWRITEDATAL = 4;
localparam okHE_REGWRITEDATAH = 35;
localparam okHE_REGREAD       = 79;

// Endpoint output harness
localparam okEH_DATAL         = 32;
localparam okEH_DATAH         = 63;
localparam okEH_READY         = 64;
localparam okEH_REGREADDATAL  = 0;
localparam okEH_REGREADDATAH  = 31;

wire        ti_clk            = okHE[okHE_CLK];
wire        ti_reset          = okHE[okHE_RESET];
wire        ti_read           = okHE[okHE_READ];
wire        ti_write          = okHE[okHE_WRITE];
wire [7:0]  ti_addr           = okHE[okHE_ADDRH:okHE_ADDRL];
wire [31:0] ti_datain         = okHE[okHE_DATAH:okHE_DATAL];
wire        ti_wireupdate     = okHE[okHE_WIREUPDATE];
wire        ti_trigupdate     = okHE[okHE_TRIGUPDATE];
wire        ti_blockstrobe    = okHE[okHE_BLOCKSTROBE];
wire [31:0] ti_reg_addr       = okHE[okHE_REGADDRH:okHE_REGADDRL];
wire        ti_reg_write      = okHE[okHE_REGWRITE];
wire [31:0] ti_reg_write_data = okHE[okHE_REGWRITEDATAH:okHE_REGWRITEDATAL];
wire        ti_reg_read       = okHE[okHE_REGREAD];