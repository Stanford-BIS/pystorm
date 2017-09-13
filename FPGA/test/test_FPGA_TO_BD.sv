`include "../src/FPGA_BD_Interface.sv"
`include "BDSrcSink.sv"

module test_FPGA_TO_BD;
// Connects FPGA channel to BD
// FPGA channel uses Valid-Acknowledge (active sender, passive receiver)
// BD uses Ready-Valid (passive sender, active receiver)
    reg reset;
    reg clk;

    Channel #(.N(`NUM_BITS_PIN2CORE)) ch_from_fpga();
    wire valid;
    wire ready;
    wire [`NUM_BITS_PIN2CORE - 1: 0] data_out;

    RandomChannelSrc #(.N(`NUM_BITS_PIN2CORE)) fpga_src(.out(ch_from_fpga), .clk(clk), .reset(reset));
    FPGA_TO_BD fpga_to_bd(.bd_channel(ch_from_fpga), .valid(valid), .data(data_out),
                          .ready(ready), .reset(reset), .clk(clk));
    BD_Sink #(.NUM_BITS(`NUM_BITS_PIN2CORE)) bd_sink(.ready(ready), .valid(valid), .data(data_out),
                                                     .reset(reset), .clk(clk));

    initial
    begin
        reset = 1;
        clk   = 0;
        #10 reset = 0;
    end

    always
        #20 clk = ~clk;
endmodule

module test_BD_TO_FPGA;
// Connects BD to FPGA channel
// FPGA channel uses Valid-Acknowledge (active sender, passive receiver)
// BD uses Ready-Valid (passive sender, active receiver)
    reg reset;
    reg clk;

    Channel #(.N(`NUM_BITS_CORE2PIN)) ch_to_fpga();
    wire valid;
    wire ready;
    wire [`NUM_BITS_CORE2PIN - 1: 0] data_in;

    ChannelSink fpga_sink(.in(ch_to_fpga), .clk(clk), .reset(reset));
    BD_TO_FPGA bd_to_fpga(.bd_channel(ch_to_fpga), .valid(valid), .data(data_in),
                          .ready(ready), .reset(reset), .clk(clk));
    BD_Source #(.NUM_BITS(`NUM_BITS_CORE2PIN)) bd_source(.ready(ready), .valid(valid), .data(data_in),
                                                         .reset(reset), .clk(clk));

    initial
    begin
        reset = 1;
        clk   = 0;
        #10 reset = 0;
    end

    always
        #20 clk = ~clk;
endmodule
