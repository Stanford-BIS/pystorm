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

