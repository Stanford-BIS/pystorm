`include "Channel.svh"

`define NUM_BITS_PIN2CORE 21
`define NUM_BITS_CORE2PIN 34

module FPGA_TO_BD #(parameter NUM_BITS = `NUM_BITS_PIN2CORE)
                   (Channel bd_channel,
                    output reg valid, output reg [NUM_BITS-1:0] data,
                    input ready, reset, clk);

// XXX this has a bug right now: needs to keep track of which
// half-cycle BD currently is in. Otherwise, metastability is possible.
// solved for now by making this clock have the same frequency as BD IO clk
// consequence is half throughput

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
logic next_a;

// data can be sampled at any time while valid is high, 
// so we want to make sure that it can't glitch
// we guard the initial transition by not asserting
// valid until a clock cycle later
always_ff @(posedge clk, posedge reset) 
  if (reset == 1) begin
    state <= WAITING_FOR_INPUT;
    data <= '0;
    bd_channel.a <= 0;
  end
  else begin
    state <= next_state;
    data <= next_data;
    bd_channel.a <= next_a;
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
        next_a = 1;
      end
      else begin
        next_data = 'X;
        next_state = WAITING_FOR_INPUT;
        next_a = 0;
      end
    end
    VALID_LOW: begin
      next_data = data; // hold data
      next_a = 0;
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
          next_a = 1;
        end
        else begin
          next_data = 'X;
          next_state = WAITING_FOR_INPUT;
          next_a = 0;
        end
      end
      else begin
        next_data = data; // hold data
        next_state = VALID_HIGH;
        next_a = 0;
      end
    end
  endcase

// output
always_comb
  case (state)
    WAITING_FOR_INPUT: begin
      valid = 0;
    end
    VALID_LOW: begin
      valid = 0;
    end
    VALID_HIGH: begin
      valid = 1;
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

  enum {READY_HIGH, READY_LOW} state, next_state;

  logic [NUM_BITS-1:0] data_int, next_data_int;

  always_ff @(posedge clk, posedge reset) 
    if (reset == 1) begin
      state <= READY_HIGH; 
      data_int <= '0;
    end
    else begin
      state <= next_state;
      data_int <= next_data_int;
    end

  always_comb
    unique case (state)
      READY_HIGH: 
        if (valid == 1) begin // data_int will be sampled, ack
          next_state = READY_LOW; 
          next_data_int = data;
        end
        else begin
          next_state = READY_HIGH;
          next_data_int = data_int;
        end
      READY_LOW: 
        if (bd_channel.a == 1) begin // can proceed if FPGA side acks
          next_state = READY_HIGH;
          next_data_int = data_int;
        end
        else begin
          next_state = READY_LOW;
          next_data_int = data_int;
        end
    endcase

  always_comb
    unique case (state)
      READY_HIGH: begin
        ready = 1;
        bd_channel.v = 0;
      end
      READY_LOW: begin
        ready = 0;
        bd_channel.v = 1;
      end
    endcase

  assign bd_channel.d = data_int;

endmodule
