`include "Channel.svh"

`define NUM_BITS_PIN2CORE 21
`define NUM_BITS_CORE2PIN 34

module FPGA_TO_BD #(parameter NUM_BITS = `NUM_BITS_PIN2CORE)
                   (Channel bd_channel,
                    output reg valid, output reg [NUM_BITS-1:0] data,
                    input ready, reset, clk);
    /*
    input bd_channel.d  // Data read from bd_channel
    input bd_channel.v  // Asserted if bd_channel data is valid
    output bd_channel.a // Assert if data sent to BD
    output data;        // Data sent to BD
    output valid;       // Assert if data sent to BD is valid
    input ready;        // Asserted if BD is ready to read
    input reset;        // Global reset
    input clk;          // Global clock
    */

    /*
    The various states are
      bd_channel.v  |  ready  ||  bd_channel.a  |  valid
            0       |    x    ||       0        |    0
            1       |    0    ||       0        |    0
            1       |    1    ||       1        |    1
    */
    enum bit {BD_VALID=1, BD_INVALID=0} STATES;
    wire state;
    assign state = bd_channel.v & ready;

    initial
    begin
        bd_channel.a   <= 0;
        valid       <= 0;
        //data        <= NUM_BITS'x;
    end

    always @(negedge clk, posedge reset)
    begin
        if(reset == 1)
        begin
            bd_channel.a   <= 0;
            valid       <= 0;
            //data        <= NUM_BITS'x;
        end
        else
        begin
            case(state)
                BD_INVALID: begin
                    bd_channel.a   <= 0;
                    valid       <= 0;
                end
                BD_VALID: begin
                    bd_channel.a   <= 1;
                    valid       <= 1;
                    data        <= bd_channel.d;
                end
            endcase
        end
    end
endmodule

module BD_TO_FPGA #(parameter NUM_BITS = `NUM_BITS_CORE2PIN)
                   (Channel bd_channel,
                    input valid, input [NUM_BITS-1:0] data,
                    output reg ready, input reset, clk);
    /*
    output channel.d  // Data sent to channel
    output channel.v  // Assert if valid data to send
    input channel.a   // At reset 0, asserted if channel.d is latched
    output ready;     // BD sends data if ready is asserted (sync)
    input valid;      // If asserted, data is valid (sync)
    input data;       // Registered data from BD (sync)
    input reset;      // Global reset (async)
    input clk;        // Global clock
    */

    /*
    The various states are
      bd_channel.a  |  valid  |  bd_channel.v  ||  bd_channel.v  |  ready
            0       |    0    |        0       ||        0       |    1
            0       |    0    |        1       ||        1       |    0
            1       |    0    |        x       ||        0       |    1
            0       |    1    |        x       ||        1       |    0
            1       |    1    |        x       ||        1       |    1
    */

    enum integer {BOTH_OFF=0, FPGA_OFF=1, BD_OFF=2, BOTH_ON=3} STATES;
    wire [1:0] state;
    assign state = {bd_channel.a, valid};

    initial
    begin
        bd_channel.v    <= 0;
        ready           <= 0;
        //data        <= NUM_BITS'x;
    end

    always @(posedge clk, posedge reset)
    begin
        if(reset == 1)
        begin
            bd_channel.v   <= 0;
            ready          <= 0;
            //data        <= NUM_BITS'x;
        end
        else
        begin
            case(state)
                BOTH_OFF: begin
                    bd_channel.v   <= bd_channel.v;
                    ready          <= ~bd_channel.v;
                end
                FPGA_OFF: begin
                    bd_channel.v   <= 1;
                    ready          <= 0;
                    bd_channel.d   <= data;
                end
                BD_OFF: begin
                    bd_channel.v   <= 0;
                    ready          <= 1;
                end
                BOTH_ON: begin
                    bd_channel.v   <= 1;
                    ready          <= 1;
                    bd_channel.d   <= data;
                end
            endcase
        end
    end
endmodule
