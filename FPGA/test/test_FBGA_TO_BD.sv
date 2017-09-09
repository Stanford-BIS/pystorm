`include "../src/FPGA_BD_Interface.sv"

module BD_Sink(ready, valid, data, reset, clk);
    parameter ClkDelaysMin = 0;
    parameter ClkDelaysMax = 5;
    parameter NUM_BITS = 32;

    output ready;
    input valid;
    input data;
    input reset;
    input clk;

    wire [NUM_BITS-1:0] data;
    reg [NUM_BITS-1:0] data_int;
    reg ready;

    int next_delay;

    initial
    begin
        ready <= 0;
        next_delay <= $urandom_range(ClkDelaysMax, ClkDelaysMin);
    end

    always @ (posedge clk or posedge reset)
    begin
        if(reset)
            ready <= 0;
        else if(next_delay > 0)
        begin
            if(valid == 1)
                ready <= 0;
            next_delay <= next_delay - 1;
        end
        else
        begin
            ready <= 1;
            next_delay <= $urandom_range(ClkDelaysMax, ClkDelaysMin);
        end
        if(valid == 1 && ready == 1)
        begin
            data_int <= data;
        end
    end
    always @ (data_int)
        $display("[T=%g]: data=%h (BD_Sink)", $time, data_int);
endmodule

module BD_Source(data, valid, ready, reset, clk);
    parameter ClkDelaysMin = 0;
    parameter ClkDelaysMax = 5;
    parameter NUM_BITS = 32;

    output data;
    output valid;
    input ready;
    input reset;
    input clk;

    reg [NUM_BITS-1:0] data;
    reg valid;

    int next_delay;

    initial
    begin
        valid <= 0;
        data  <= 0;
        next_delay <= $urandom_range(ClkDelaysMax, ClkDelaysMin);
    end

    always @ (posedge clk or posedge reset)
    begin
        if(reset || ready == 0)
            valid <= 0;
        else if(next_delay > 0)
        begin
            if(ready == 1)
                valid <= 0;
            next_delay <= next_delay - 1;
        end
        else
        begin
            next_delay <= $urandom_range(ClkDelaysMax, ClkDelaysMin);
            if(ready == 1)
            begin
                valid <= 1;
                data <= data + 1;
            end
        end
    end
    always @ (data)
        $display("[T=%g]: data=%h (BD_Source)", $time, data);
endmodule

module test_BD_Source_Sink;
// Connect BD Source to BD Sink
// BD has Ready-Valid protocol
    reg reset;
    reg clk;

    wire valid;
    wire ready;
    wire [32 - 1: 0] data;

    BD_Source src(data, valid, ready, reset, clk);
    BD_Sink  sink(ready, valid, data, reset, clk);

    initial
    begin
        reset = 1;
        clk   = 0;
        #10 reset = 0;
    end

    always
        #20 clk = ~clk;
endmodule

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