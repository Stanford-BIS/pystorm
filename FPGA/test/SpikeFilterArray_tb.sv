`include "../src/SpikeFilterArray.sv"
`include "ChannelSrcSink.sv"

module SpikeFilterArray_tb;

parameter Nfilts = 10;
parameter Nstate = 27;
parameter Nct = 10;

SpikeFilterOutputChannel out();

TagCtChannel #(.Ntag(Nfilts), .Nct(Nct)) in();

SpikeFilterConf #(Nfilts, Nstate) conf();

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

// update_pulse
parameter ClksPerUpdate = 64;
logic update_pulse;
initial begin
  update_pulse <= 0;
  forever begin
    #(ClksPerUpdate*Tclk) @(posedge clk) update_pulse <= 1;
    @(posedge clk) update_pulse <= 0;
  end
end

// receiver
Channel #(Nstate + Nfilts) packed_out();
assign packed_out.v = out.v;
assign packed_out.d = {out.filt_idx, out.filt_state};
assign out.a = packed_out.a;
ChannelSink #(.ClkDelaysMin(0), .ClkDelaysMax(10)) out_sink(packed_out, clk, reset);

// stimulus
initial begin
  @(posedge reset)
  // "count mode"
  conf.increment_constant <= 1;
  conf.decay_constant <= 0;

  // filts initially off
  conf.filts_used <= 0;

  // send some tags, make sure nothing happens
  #(Tclk * 10) 
  @(posedge clk) 
  in.v <= 1;
  in.tag <= 0;
  in.ct <= 1;
  @(posedge clk) 
  in.v <= 0;
  in.tag <= 'X;
  in.ct <= 'X;

  #(Tclk * 10) 
  @(posedge clk) 
  in.v <= 1;
  in.tag <= 1;
  in.ct <= 1;
  @(posedge clk) 
  in.v <= 0;
  in.tag <= 'X;
  in.ct <= 'X;

  // turn 2 filts on
  conf.filts_used <= 2;

  // send tags
  repeat(10) begin
    #(Tclk * 10) 
    @(posedge clk);
    in.v <= 1;
    in.tag <= 0;
    in.ct <= 1;
    forever begin 
      @(posedge clk);
      if (in.a == 1) begin
        in.v <= 0;
        in.tag <= 'X;
        in.ct <= 'X;
        break;
      end
    end

    #(Tclk * 10) 
    @(posedge clk);
    in.v <= 1;
    in.tag <= 1;
    in.ct <= 2;
    forever begin 
      @(posedge clk);
      if (in.a == 1) begin
        in.v <= 0;
        in.tag <= 'X;
        in.ct <= 'X;
        break;
      end
    end
  end

  // do the same thing in "decay mode"
  // using numbers from example
  #(Tclk * 10) 
  conf.increment_constant <= 5120;
  conf.decay_constant <= 134083577;

  // send tags
  repeat(10) begin
    #(Tclk * 10) 
    @(posedge clk);
    in.v <= 1;
    in.tag <= 0;
    in.ct <= 1;
    forever begin 
      @(posedge clk);
      if (in.a == 1) begin
        in.v <= 0;
        in.tag <= 'X;
        in.ct <= 'X;
        break;
      end
    end

    #(Tclk * 10) 
    @(posedge clk);
    in.v <= 1;
    in.tag <= 1;
    in.ct <= 2;
    forever begin
      @(posedge clk);
      if (in.a == 1) begin
        in.v <= 0;
        in.tag <= 'X;
        in.ct <= 'X;
        break;
      end
    end
  end

end

SpikeFilterArray dut(.*);

endmodule
