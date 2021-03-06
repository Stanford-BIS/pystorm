// the unit of time is 1-64K clock cycles, user-configurable
module TimeUnitPulser #(parameter N = 16) ( // 2^N = time unit max val, in clock cycles
  output logic unit_pulse, 
  input [N-1:0] clks_per_unit,
  input clk, 
  input reset);

logic [N-1:0] count;
logic [N-1:0] count_p1;

assign count_p1 = count + 1;

always @(posedge clk, posedge reset) begin
  if (reset == 1) begin
    count <= 1;
    unit_pulse <= 0;
  end
  else begin
    if (count >= clks_per_unit) begin
      unit_pulse <= 1;
      count <= 1;
    end
    else begin
      unit_pulse <= 0; 
      count <= count_p1;
    end
  end
end

endmodule
