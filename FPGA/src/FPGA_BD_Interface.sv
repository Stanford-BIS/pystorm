`include "Channel.svh"

`define NUM_BITS_PIN2CORE 21
`define NUM_BITS_CORE2PIN 34

module FPGA_TO_BD #(parameter NUM_BITS = `NUM_BITS_PIN2CORE)
                   (Channel bd_channel,
                    output reg valid, output reg [NUM_BITS-1:0] data,
                    input ready, reset, clk);

/*
input bd_channel.d  // Data read from bd_channel (from the FPGA core)
input bd_channel.v  // Asserted if bd_channel data is valid
output bd_channel.a // Assert if data sent to BD
output data;        // Data sent to BD
output valid;       // Assert if data sent to BD is valid
input ready;        // Asserted if BD is ready to read
input reset;        // Global reset
input clk;          // Global clock
*/

enum {WAITING_FOR_INPUT, VALID_LOW, VALID_HIGH} state, next_state;
logic [NUM_BITS-1:0] next_data;

// data can be sampled at any time while valid is high, 
// so we want to make sure that it can't glitch
// we guard the initial transition by not asserting
// valid until a clock cycle later
always_ff @(posedge clk, posedge reset) 
  if (reset == 1) begin
    state <= WAITING_FOR_INPUT;
    data <= '0;
  end
  else begin
    state <= next_state;
    data <= next_data;
  end

// state update/data update
// have to be a bit careful, ready can come back any time
// this is the only place we can look at ready
always_comb
  case (state)
    WAITING_FOR_INPUT: begin
      if (bd_channel.v == 1) begin
        next_data = bd_channel.d; // set up new data
        next_state = VALID_LOW;
      end
      else begin
        next_data = 'X;
        next_state = WAITING_FOR_INPUT;
      end
    end
    VALID_LOW: begin
      next_data = data; // hold data
      if (ready == 1)
        next_state = VALID_HIGH;
      else
        next_state = VALID_LOW;
    end
    VALID_HIGH: begin
      if (ready == 0) begin
        if (bd_channel.v == 1) begin
          next_data = bd_channel.d; // set up new data
          next_state = VALID_LOW;
        end
        else begin
          next_data = 'X;
          next_state = WAITING_FOR_INPUT;
        end
      end
      else begin
        next_data = data; // hold data
        next_state = VALID_HIGH;
      end
    end
  endcase

// output
always_comb
  case (state)
    WAITING_FOR_INPUT: begin
      valid = 0;
      if (bd_channel.v == 1)
        bd_channel.a = 1;
      else
        bd_channel.a = 0;
    end
    VALID_LOW: begin
      valid = 0;
      bd_channel.a = 0;
    end
    VALID_HIGH: begin
      valid = 1;
      if (bd_channel.v == 1)
        bd_channel.a = 1;
      else
        bd_channel.a = 0;
    end
  endcase

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
            1       |    1    |        x       ||        1       |    0
    */

    reg slow_valid;

    typedef enum {
      BOTH_OFF = 0,
      FPGA_OFF = 1,
      BD_OFF   = 2,
      BOTH_ON  = 3} stateType;

    stateType state;
    assign state = stateType'({bd_channel.a, valid});

    always_ff @(posedge clk, posedge reset)
    begin
        if(reset == 1)
        begin
            slow_valid     <= 0;
            ready          <= 1;
        end
        else
        begin
            case(state)
                //BOTH_OFF: begin
                //    bd_channel.v   <= bd_channel.v;
                //    ready          <= ~bd_channel.v;
                //end
                FPGA_OFF: begin
                    ready          <= 0;
                    if(ready)
                    begin
                        slow_valid     <= 1;
                        bd_channel.d   <= data;
                    end
                end
                BD_OFF: begin
                    slow_valid     <= 0;
                    ready          <= 1;
                end
                BOTH_ON: begin
                    ready          <= 0;
                    if(ready)
                    begin
                        slow_valid     <= 1;
                        bd_channel.d   <= data;
                    end
                end
            endcase
        end
    end

    assign bd_channel.v = (valid & ready & bd_channel.a) ? 1 : slow_valid;
endmodule
