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

wire tail;
wire [9:0] dest;
//SUBJECT TO CHANGE. HAVE TO DECIDE ON DATA FORMAT
assign dest=data_in[9:0];
assign tail = data_in[10];
////////////////////////////////////////////

reg [1:0] state=2'b00;  
reg [1:0] next_state;
parameter parse = 2'b00; 
parameter send_0 = 2'b01;
parameter send_1 = 2'b10;

//Next State combinational logic
always @ (state or tail or empty or dest or ready_0 or ready_1)
begin : next_state_logic
	next_state = state;
	case(state)					
		parse:	begin
						if($signed(dest)>0 && ready_1==1) begin
							next_state=send_1;
						end
						else if ($signed(dest)<=0 && ready_0==1) begin
							next_state=send_0;
						end
					end
							
		send_0:	begin
						if((tail==1)) begin
							next_state=parse;
						end
					end
		
		send_1:	begin
						if(tail==1) begin
							next_state=parse;
						end
					end
		
		default:	next_state=state;
	
	endcase
end

always @ (posedge clk)
	begin: state_register
	state <= next_state;
end

//output combinational logic
always @ (state or ready_0 or ready_1 or tail or dest or data_in or empty)
begin: output_logic
	case(state)

		parse:	begin
						if($signed(dest) > 0) begin
							req_0=0;
							//If we have a valid header with a positive destination field, send it to output 1.
							req_1=~empty;
							//decrement hop count in header.
							data_out={data_in[10],dest-10'b0000000001};
							read=~empty & ready_1;
						end
						else begin
							//If we have a valid header with a dest field <= 0, send it to output 0
							req_0=~empty;
							req_1=0;
							//negate destination if going to output 0
							data_out={data_in[10],-dest};
							read=~empty & ready_0;
						end
							
					end
						
		send_0:	begin
						req_0=~empty;
						req_1=0;
						read = ~empty && ready_0; //read as long as output is ready to receive and input FIFO not empty
						data_out=data_in;
					end
				
		send_1:	begin
						req_0=0;
						req_1=~empty;
						read = ~empty && ready_1;
						data_out=data_in;
					end
	endcase
end

endmodule