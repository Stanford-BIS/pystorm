//Input Controller

//In each router node, there is one input controller per input.
//In controls the input FIFOs and based on the destination field
//of the input header, request the proper output.

module InputController (
input clk,
input empty,
input ready_0,
input ready_1,
input [10:0] data_in,
output reg req_0,
output reg req_1,
output reg read,
output reg [10:0] data_out
);

reg [10:0] data_in_ff=0;
reg [10:0] data_out_ff=0;
reg empty_ff = 1;
reg final_empty = 1;
wire tail;
wire [9:0] dest;
//SUBJECT TO CHANGE. HAVE TO DECIDE ON DATA FORMAT
assign dest = data_in_ff[9:0];
assign tail = data_out[10];
////////////////////////////////////////////

reg [1:0] state=2'b00;  
reg [1:0] next_state;
parameter idle = 2'b00;
parameter parse = 2'b01; 
parameter send_0 = 2'b10;
parameter send_1 = 2'b11;

//FF with Enable for data_in_ff, empty
always @ (posedge clk) begin
	if (read) begin
		data_in_ff <= data_in;
		empty_ff <= empty;
		final_empty <= empty_ff;
		data_out <= data_out_ff;
	end
end

//Next State combinational logic
always @ (state or tail or empty or dest or ready_0 or ready_1)
begin : next_state_logic
	next_state = state;
	case(state)		
		idle:		begin
						if(~empty)
							next_state = parse;
					end
	
		parse:	begin
						if($signed(dest)>0) begin
							next_state = send_1;
						end
						else if ($signed(dest)<=0) begin
							next_state = send_0;
						end
					end
							
		send_0:	begin
						if(tail && ~empty_ff && ready_0) begin
							next_state = parse;
						end
						else if (tail && ready_0) begin
							next_state = idle;
						end
					end
		
		send_1:	begin
						if(tail && ~empty_ff && ready_1) begin
							next_state = parse;
						end
						else if(tail && ready_1) begin
							next_state = idle;
						end
					end
		
		default:	next_state = state;
	
	endcase
end

always @ (posedge clk)
	begin: state_register
	state <= next_state;
end

//output combinational logic
always @ (state or ready_0 or ready_1 or tail or dest or data_in_ff or empty or final_empty)
begin: output_logic
	case(state)

		idle:		begin
						req_0 = 0;
						req_1 = 0;
						data_out_ff = 0;
						read = ~empty;
					end	
						
		parse:	begin
						if($signed(dest) > 0) begin
							req_0 = 0;
							//If we have a valid header with a positive destination field, send it to output 1.
							req_1 = 0;
							//decrement hop count in header.
							data_out_ff = {data_in_ff[10],dest-10'b0000000001};
							read = 1;
						end
						else begin
							//If we have a valid header with a dest field <= 0, send it to output 0
							req_0 = 0;
							req_1 = 0;
							//negate destination if going to output 0
							data_out_ff = {data_in_ff[10],-dest};
							read = 1;
						end
							
					end
						
		send_0:	begin
						req_0 = ~final_empty;
						req_1 = 0;
						read = ready_0 && ~tail; //read as long as output is ready to receive
						data_out_ff = data_in_ff;
					end
				
		send_1:	begin
						req_0=0;
						req_1= ~final_empty;
						read = ready_1 && ~tail;
						data_out_ff = data_in_ff;
					end
	endcase
end

endmodule