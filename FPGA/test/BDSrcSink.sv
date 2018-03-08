`include "ChannelSrcSink.sv"

module CElement(output logic c, input a, b, reset, reset_val);
  always @(a, b, reset)
    if (reset == 1)
      c = reset_val;
    else if (a == 1 && b == 1)
      c = 1;
    else if (a == 0 && b == 0)
      c = 0;
endmodule


module CElement_(output logic c, input a, b, reset, reset_val);
  logic _c, _reset_val;
  assign _reset_val = ~reset_val;
  assign c = ~_c;
  CElement base(_c, a, b, reset, _reset_val);
endmodule


module Arbiter(output logic u, v, input a, b, reset);

  // for fairness
  logic last_picked = 0;

  always @(a, b, u, v, reset)
    if (reset == 1) begin
      u = 0;
      v = 0;
    end
    else
      if (u == 0 && v == 0) begin
        if (a == 1 && b ==1) begin
          if (last_picked == 0)
            v = 1;
          else
            u = 1;
          last_picked = ~last_picked;
        end
        else if (a == 1)
          u = 1;
        else if (b == 1)
          v = 1;
        else begin
          u = 0;
          v = 0;
        end
      end

      else if (u == 1) begin
        if (a == 0)
          u = 0;
      end

      else if (v == 1) begin
        if (b == 0)
          v = 0;
      end

      else if (u == 1 && v == 1) begin
        assert(0);
      end

endmodule

interface SyncIntf #(parameter N = 1);
  logic [N-1:0] D;
  logic d;
  logic e;
endinterface


module SynchronizerStage #(parameter N = 1) (
  // output side
  SyncIntf out,
  // input side
  SyncIntf in,
  input clk, reset);

  logic en;
  logic sk; // unused clk arbiter output

  CElement_ ine_c(in.e, in.d, out.e, reset, 1'b1);

  assign en = ~in.e;

  Arbiter arb(out.d, sk, en, clk, reset);

  // DFF for data
  always @(posedge en, posedge reset)
    for (int i = 0; i < N; i++)
      if (reset == 1)
        out.D[i] = 0;
      else
        out.D[i] = in.D[i];
endmodule

module Synchronizer #(parameter N = 1, M = 1) (
  // output side
  output logic [N-1:0] DO,
  output logic outd,
  input logic oute,
  // input side
  input logic [N-1:0] DI,
  input logic ind,
  output logic ine,
  input clk, reset);

  // has M*2 stages

  SyncIntf #(N) stage_intf[2*M:0] ();

  logic _clk;
  assign _clk = ~clk;

  // hook up signals
  generate
  for (genvar i = 0; i < M; i++) begin
    SynchronizerStage #(N) stage_even(
      .out(stage_intf[2*i+1]),
      .in(stage_intf[2*i]),
      .reset(reset),
      .clk(clk));

    SynchronizerStage #(N) stage_odd(
      .out(stage_intf[2*i+2]),
      .in(stage_intf[2*i+1]),
      .reset(reset),
      .clk(_clk));
  end
  endgenerate

  assign stage_intf[0].D = DI;
  assign stage_intf[0].d = ind;
  assign ine = stage_intf[0].e;

  assign DO = stage_intf[2*M].D;
  assign outd = stage_intf[2*M].d;
  assign stage_intf[2*M].e = oute;

endmodule


module BD_Sink #(
  parameter DelayMin = 0,
  parameter DelayMax = 200,
  parameter NUM_BITS = 21) (
  output logic ready,
  input valid,
  input [NUM_BITS-1:0] data,
  input reset, clk);

  logic [NUM_BITS-1:0] sync_DO;
  logic                sync_oute;
  logic                sync_outd;

  Synchronizer #(NUM_BITS, 2) sync(
    .DO(sync_DO),
    .outd(sync_outd),
    .oute(sync_oute),
    .DI(data),
    .ind(valid),
    .ine(ready),
    .clk(clk),
    .reset(reset));

  int delay;

  // BD internal handshake
  always @(sync_outd, posedge reset)
    if (reset == 1)
      sync_oute = 1;
    else
      if (sync_outd == 1) begin
        $display("[T=%g]: data=%h (BD_Sink)", $time, sync_DO);
        delay = $urandom_range(DelayMax, DelayMin);
        for (int i = 0; i < delay + 1; i++) begin
          #(1);
        end
        sync_oute = 0;
      end
      else begin // (sync_outd == 0)
        delay = $urandom_range(DelayMax, DelayMin);
        for (int i = 0; i < delay + 1; i++) begin
          #(1);
        end
        sync_oute = 1;
      end

endmodule


module BD_Source #(
  parameter DelayMin = 0,
  parameter DelayMax = 200,
  parameter NUM_BITS = 34) (
  output logic[NUM_BITS-1:0] data,
  output logic               _valid,
  input                      ready,
  input                      reset,
  input                      clk);

  logic [NUM_BITS-1:0] sync_DI;
  logic                sync_ine;
  logic                sync_ind;

  logic                sync_oute;
  logic                sync_outd;

  Synchronizer #(NUM_BITS, 2) sync(
    .DO(data),
    .outd(sync_outd),
    .oute(sync_oute),
    .DI(sync_DI),
    .ind(sync_ind),
    .ine(sync_ine),
    .clk(clk),
    .reset(reset));

  CElement_ _valid_cel(_valid, ready, sync_outd, reset, 1'b1);
  assign sync_oute = ready;

  int delay;
  
  parameter Nfunnel = 13;
  parameter NBDdata = NUM_BITS;
  const logic [0:Nfunnel-1][NUM_BITS-1:0] routes = '{
    'b100100000000000 << (NBDdata - 15),
    'b100100000000001 << (NBDdata - 15),
    'b10010000000001  << (NBDdata - 14),
    'b100100000001100 << (NBDdata - 15),
    'b100100000001101 << (NBDdata - 15),
    'b10010000000101  << (NBDdata - 14),
    'b10000           << (NBDdata - 5 ),
    'b10001           << (NBDdata - 5 ),
    'b101             << (NBDdata - 3 ),
    'b10010000000100  << (NBDdata - 14),
    'b10010000000100  << (NBDdata - 14),
    'b01              << (NBDdata - 2 ),
    'b00              << (NBDdata - 2 )};

  const logic [0:Nfunnel-1][NBDdata-1:0] route_masks = '{
    {{15{1'b1}}, {(34-15){1'b0}}},
    {{15{1'b1}}, {(34-15){1'b0}}},
    {{14{1'b1}}, {(34-14){1'b0}}},
    {{15{1'b1}}, {(34-15){1'b0}}},
    {{15{1'b1}}, {(34-15){1'b0}}},
    {{14{1'b1}}, {(34-14){1'b0}}},
     {{5{1'b1}},  {(34-5){1'b0}}},
     {{5{1'b1}},  {(34-5){1'b0}}},
     {{3{1'b1}},  {(34-3){1'b0}}},
    {{14{1'b1}}, {(34-14){1'b0}}},
    {{14{1'b1}}, {(34-14){1'b0}}},
     {{2{1'b1}},  {(34-2){1'b0}}},
     {{2{1'b1}},  {(34-2){1'b0}}}};

  int funnel_idx;
  logic [NUM_BITS-1:0] funnel_route;
  logic [NUM_BITS-1:0] payload_mask;
  logic [63:0] big_rand;
  logic [NUM_BITS-1:0] masked_payload;
    
  // BD internal handshake
  always @(sync_ine, reset)
    if (reset == 1) begin
      sync_ind = 0;
      sync_DI = 0;
    end
    else
      if (sync_ine == 1) begin
        funnel_idx = 0;
        funnel_route = routes[funnel_idx];
        payload_mask = ~route_masks[funnel_idx];

        big_rand = {$urandom_range(0,2**32-1), $urandom_range(0,2**32-1)};

        masked_payload = payload_mask & big_rand[NUM_BITS-1:0];
        sync_DI = funnel_route | masked_payload;

        $display("[T=%g]: route_idx=%b (BD_Source)", $time, funnel_idx);
        $display("[T=%g]: route=%b (BD_Source)", $time, funnel_route);
        $display("[T=%g]: masked_payload=%b (BD_Source)", $time, masked_payload);
        $display("[T=%g]: data=%b (BD_Source)", $time, sync_DI);

        assert(masked_payload ^ funnel_route == 0);

        delay = $urandom_range(DelayMax, DelayMin);
        for (int i = 0; i < delay + 1; i++) begin
          #(1);
        end
        sync_ind = 1;
      end
      else if (sync_ine == 0) begin
        delay = $urandom_range(DelayMax, DelayMin);
        for (int i = 0; i < delay + 1; i++) begin
          #(1);
        end
        sync_ind = 0;
      end

endmodule

module BD_Sink_tb;

  parameter DelayMin = 0;
  parameter DelayMax = 200;
  parameter NUM_BITS = 21;

  logic ready;
  logic valid;
  logic [NUM_BITS-1:0] data;

  logic ready_2;
  logic valid_2;
  logic [NUM_BITS-1:0] data_2;

  // clock
  logic clk;
  parameter Tclk = 100;
  always #(Tclk/2) clk = ~clk;
  initial clk = 0;

  logic clk_2;
  parameter Tclk_2 = 50;
  always #(Tclk_2/2) clk_2 = ~clk_2;
  initial clk_2 = 1;

  // reset
  logic reset;
  initial begin
    reset <= 0;
    @(posedge clk) reset <= 1;
    @(posedge clk) reset <= 0;
  end

  // skewed clock for testing setup violations on ready
  logic skewed_clk;
  parameter Tskew = 80;
  initial begin
    #(Tskew) skewed_clk = 0;
    forever
      #(Tclk/2) skewed_clk = ~skewed_clk;
  end

  logic ready_at_skewed;
  always @(skewed_clk)
    ready_at_skewed = ready;

  always @(clk, posedge reset)
    if (reset == 1) begin
      valid = 0;
      data = 0;
    end
    else begin
      $display("[T=%g]: valid=%h, clk=%h", $time, valid, clk);

      // full data rate, but ready can show up near the clock edge
      if (clk == 0 && ready == 1) begin
        if (ready != ready_at_skewed)
          $display("ready changed close to negedge");
          if(~valid)
          begin
            valid = 1;
            data = data + 1;
          end
      end
      else if (clk == 1 && ready == 0) begin
        if (ready != ready_at_skewed)
          $display("ready changed close to posedge");
        valid = 0;
      end
    end

    always @ (posedge clk_2 or posedge reset)
    begin
        if(reset)
        begin
            valid_2 <= 0;
            data_2 <= 0;
        end
        else if(ready_2)
        begin
          if(~valid_2)
          begin
            valid_2 <= 1;
            data_2 <= data_2 + 1;
          end
        end
        else if(~ready_2)
            valid_2 <= 0;
    end

 BD_Sink #(.DelayMin(DelayMin), .DelayMax(DelayMax)) dut(.*);
 BD_Sink #(.DelayMin(DelayMin), .DelayMax(DelayMax)) dut_2(.ready(ready_2), .valid(valid_2), .data(data_2),
  .clk(clk), .reset(reset));

endmodule

module BD_Source_tb;

  parameter DelayMin = 0;
  parameter DelayMax = 200;
  parameter NUM_BITS = 34;

  logic[NUM_BITS-1:0] data;
  logic               _valid;
  logic               ready;

  logic[NUM_BITS-1:0] data_2;
  logic               _valid_2;
  logic               ready_2;

  // clock
  logic clk;
  parameter Tclk = 100;
  always #(Tclk/2) clk = ~clk;
  initial clk = 0;

  logic clk_2;
  parameter Tclk_2 = 50;
  always #(Tclk_2/2) clk_2 = ~clk_2;
  initial clk_2 = 1;

  // reset
  logic reset;
  initial begin
    reset <= 0;
    @(posedge clk) reset <= 1;
    @(posedge clk) reset <= 0;
  end

  BD_Source #(.DelayMin(DelayMin), .DelayMax(DelayMax)) dut(.*);
  BD_Source #(.DelayMin(DelayMin), .DelayMax(DelayMax)) dut_2(.ready(ready_2), ._valid(_valid_2), .data(data_2),
    .clk(clk), .reset(reset));

  always @(clk, reset)
    if (reset == 1)
      ready = 1;
    else
      if (clk == 0 && _valid == 0) begin
        if(ready)
        begin
          ready = 0;
          $display("[T=%g]: data=%h (testbench sink)", $time, data);
        end
      end
      else if (clk == 1 && _valid == 1)
        ready = 1;

  always @ (posedge clk_2 or posedge reset)
  begin
    if(reset)
      ready_2 <= 1;
    else if(~_valid_2)
    begin
      if(ready_2)
      begin
        ready_2 <= 0;
        $display("[T=%g]: data=%h (testbench sink_2)", $time, data_2);
      end
    end
    else if(_valid_2)
      ready_2 <= 1;
  end
endmodule

