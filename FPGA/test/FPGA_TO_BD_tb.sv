`define SIMULATION
`include "../src/FPGA_BD_Interface.sv"
`include "ChannelSrcSink.sv"
`include "BDSrcSink.sv"

module FPGA_TO_BD_tb;
// Connects FPGA channel to BD
// FPGA channel uses Valid-Acknowledge (active sender, passive receiver)
// BD uses Ready-Valid (passive sender, active receiver)
    parameter DelayMinCh = 0;
    parameter DelayMaxCh = 5;
    // parameter DelayMaxCh = 0;
    parameter DelayMinBD = 0;
    parameter DelayMaxBD = 200;
    // parameter DelayMaxBD = 0;
    reg reset;
    reg clk;
    reg clk2;

    Channel #(.N(`NUM_BITS_PIN2CORE)) ch_from_fpga();
    wire valid;
    wire ready;
    wire [`NUM_BITS_PIN2CORE - 1: 0] data_out;

    RandomChannelSrc #(.N(`NUM_BITS_PIN2CORE), .ClkDelaysMin(DelayMinCh), .ClkDelaysMax(DelayMaxCh)) fpga_src(.out(ch_from_fpga), .clk(clk2), .reset(reset));
    FPGA_TO_BD fpga_to_bd(.bd_channel(ch_from_fpga), .valid(valid), .data(data_out),
                          .ready(ready), .reset(reset), .clk(clk2));
    BD_Sink #(.NUM_BITS(`NUM_BITS_PIN2CORE), .DelayMin(DelayMinBD), .DelayMax(DelayMaxBD)) bd_sink(.ready(ready), .valid(valid), .data(data_out), .reset(reset), .clk(clk));

    initial
    begin
        reset = 1;
        clk   = 0;
        clk2   = 1;
        #10 reset = 0;
    end

    always
        #20 clk = ~clk;

    always
        #10 clk2 = ~clk2;
endmodule

module BD_TO_FPGA_tb;
// Connects BD to FPGA channel
// FPGA channel uses Valid-Acknowledge (active sender, passive receiver)
// BD uses Ready-Valid (passive sender, active receiver)
    parameter DelayMinCh = 0;
    parameter DelayMaxCh = 5;
    // parameter DelayMaxCh = 0;
    parameter DelayMinBD = 0;
    parameter DelayMaxBD = 200;
    // parameter DelayMaxBD = 0;
    reg reset;
    reg clk;
    reg clk2;

    Channel #(.N(`NUM_BITS_CORE2PIN)) ch_to_fpga();
    wire valid;
    wire _valid;
    wire ready;
    wire [`NUM_BITS_CORE2PIN - 1: 0] data_in;

    assign valid = ~_valid;

    ChannelSink #(.ClkDelaysMin(DelayMinCh), .ClkDelaysMax(DelayMaxCh)) fpga_sink(.in(ch_to_fpga), .clk(clk), .reset(reset));
    BD_TO_FPGA bd_to_fpga(.bd_channel(ch_to_fpga), .valid(valid), .data(data_in),
                          .ready(ready), .reset(reset), .clk(clk2));
    BD_Source #(.NUM_BITS(`NUM_BITS_CORE2PIN), .DelayMin(DelayMinBD), .DelayMax(DelayMaxBD)) bd_source(.ready(ready), ._valid(_valid), .data(data_in), .reset(reset), .clk(clk));

    initial
    begin
        reset = 1;
        clk   = 0;
        clk2   = 1;
        #10 reset = 0;
    end

    always
        #20 clk = ~clk;

    always
        #10 clk2 = ~clk2;
endmodule
