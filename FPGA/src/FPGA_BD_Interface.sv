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

enum {READY, READY_ACK, SENDING} state, next_state;

always_ff @(posedge clk, posedge rst) 
  if (reset == 1)
    state <= READY;
  else
    state <= next_state;
 
// we're clocked at twice the rate of BD, so we can be a little sloppy with the
// Channel handshake (don't have ack in same cycle as valid)

// have to be a bit careful, ready can come back any time
// only look at ready in state update and make output a function purely of state

// state update
always_comb
  case (state)
    READY:
      if (bd_channel.v == 1)
        next_state = SENDING;
      else
        next_state = READY;
    READY_ACK:
      if (bd_channel.v == 1)
        next_state = SENDING;
      else
        next_state = READY;
    SENDING:
      if (ready == 0) // this is the only place we look at ready
        next_state = READY_ACK;
      else
        next_state = SENDING;
  endcase

// output
always_comb
  case (state)
    READY: begin
      valid = 0;
      bd_channel.a = 0;
      data = '0;
    end
    READY_ACK: begin
      valid = 0;
      bd_channel.a = 1;
      data = '0;
    end
    SENDING: begin
      valid = 1;
      data = bd_channel.d;
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
