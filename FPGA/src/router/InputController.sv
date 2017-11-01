//Controls input FIFOs

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
parameter idle = 2'b00; 
parameter parse = 2'b01; 
parameter send_0 = 2'b10;
parameter send_1 = 2'b11;

//Next State combinational logic
always @ (state or tail or empty or dest or ready_0 or ready_1)
begin : next_state_logic
	next_state = state;
	case(state)
		idle:		begin
						if(empty==0)
							next_state=parse;
					end	
						
		parse:	begin
						if($signed(dest)>0 && ready_1==1) begin
							next_state=send_1;
						end
						else if ($signed(dest)<=0 && ready_0==1) begin
							next_state=send_0;
						end
					end
							
		send_0:	begin
						if((tail==1) && (empty==1)) begin
							next_state=idle;
						end
						else if((tail==1) && (empty==0)) begin
							next_state=parse;
						end
					end
		
		send_1:	begin
						if((tail==1) && (empty==1)) begin
							next_state=idle;
						end
						else if((tail==1) && (empty==0)) begin
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
		idle:		begin
						read=0;
						req_0=0;
						req_1=0;
						data_out=data_in;
					end

		parse:	begin
						if($signed(dest) > 0) begin
							req_0=0;
							req_1=1;
							//decrement hop count in header.
							data_out={data_in[10],dest-10'b1};
							read=~empty & ready_1;
						end
						else begin
							req_0=1;
							req_1=0;
							//negate destination if going out the bottom
							data_out={data_in[10],-dest};
							read=~empty & ready_0;
						end
							
					end
						
		send_0:	begin
						req_0=~empty;
						req_1=0;
						read = ~empty && ready_0;
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