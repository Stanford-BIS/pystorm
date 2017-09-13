`include "../src/FPGASerializer.sv"
`include "ChannelSrcSink.sv"

module FPGASerializer_tb;

parameter NPCcode = 8;
parameter NPCdata = 24;
parameter Ntime = 48;
parameter N_SF_filts = 10;
parameter N_SF_state = 27;

// our only output goes to the PC
SerializedPCWordChannel PC_out();

// Filtered spikes
SpikeFilterOutputChannel #(N_SF_filts, N_SF_state) SF_in();

// clock
logic clk;
parameter Tclk = 10;
always #(Tclk/2) clk = ~clk;
initial clk = 0;

// reset
logic reset;
initial begin
  reset <= 0;
  @(posedge clk) reset <= 1;
  @(posedge clk) reset <= 0;
end

// HB pulse source
logic send_HB_up_pulse; // pulses every send_HB_up_every * time_unit; for 1 clk cycle 
logic [Ntime-1:0] time_elapsed; // current wall time; can pack into unused spike bits for higher resolution

parameter ClksPerHB = 64;
initial begin
  send_HB_up_pulse <= 0;
  time_elapsed <= 0; // we're just going to count pulses
  forever begin
    #(ClksPerHB*Tclk) @(posedge clk) send_HB_up_pulse <= 1;
    @(posedge clk);
    send_HB_up_pulse <= 0;
    time_elapsed <= time_elapsed + 1;
  end
end

// SF sender
Channel #(N_SF_filts + N_SF_state) SF_in_packed();
assign SF_in_packed.a = SF_in.a;
assign {SF_in.filt_idx, SF_in.filt_state} = SF_in_packed.d;
assign SF_in.v = SF_in_packed.v;
RandomChannelSrc #(.N(N_SF_filts + N_SF_state), .ClkDelaysMin(0), .ClkDelaysMax(10)) SF_in_src(SF_in_packed, clk, reset);

// output sink
Channel #(NPCcode + NPCdata) PC_out_packed();
assign PC_out_packed.v = PC_out.v;
assign PC_out_packed.d = {PC_out.code, PC_out.payload};
assign PC_out.a = PC_out_packed.a;
ChannelSink #(.ClkDelaysMin(0), .ClkDelaysMax(2)) PC_out_sink(PC_out_packed, clk, reset);

FPGASerializer dut(.*);

endmodule
