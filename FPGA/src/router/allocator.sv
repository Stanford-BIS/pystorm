//output Allocator

module allocator (
input clk,
input req_0,
input req_1,
input out_FIFO_full,
input [10:0] data_in_0,
input [10:0] data_in_1,
output reg ready_0,
output reg ready_1,
output reg out_FIFO_wr,
output reg [10:0] data_out);

wire tail_0;
wire tail_1;
assign tail_0 = data_in_0[10];
assign tail_1 = data_in_1[10];
reg req_0_ff;
reg req_1_ff;


//4 states
reg [1:0] state = 2'b00;  
reg [1:0] next_state;

parameter idle_recent_0 = 2'b00; 
parameter msg_0 = 2'b01;  
parameter msg_1 = 2'b10; 
parameter idle_recent_1 = 2'b11;


//State Register
always @ (posedge clk)
	begin: state_register
	state <= next_state;
end

//Next State combinational logic
always @ (state or tail_0 or tail_1 or req_0 or req_1)
begin : next_state_logic
	next_state = state;
	case(state)
		idle_recent_0:	begin
						if(req_1==1)
							next_state=msg_1;
						else if(req_0==1)
							next_state=msg_0;
						end
						
		idle_recent_1:	begin
						if(req_0==1)
							next_state=msg_0;
						else if(req_1==1)
							next_state=msg_1;
						end
						
		msg_0:			begin
						if((tail_0==1) && (req_1==1))
							next_state=msg_1;
						else if(tail_0==1)
							next_state=idle_recent_0;
						end
							
		msg_1:			begin
						if((tail_1==1) && (req_0==1))
							next_state=msg_0;
						else if(tail_1==1)
							next_state=idle_recent_1;
						end
		
		default:		next_state=state;
	
	endcase
end


//Flip Flops for request signal. Needed to control output FIFO_wr when input FIFO becomes empty.
//always @ (posedge clk)
//	begin: req_FFs
//	req_0_ff <= req_0;
//	req_1_ff <= req_1;
//end

//output combinational logic
always @ (state or out_FIFO_full or data_in_0 or data_in_1 or req_0 or req_1)
begin: output_logic
	case(state)
		idle_recent_0:	begin
						ready_0 = 0;
						ready_1 = 0;
						data_out=data_in_0;
						out_FIFO_wr=0;
						end
						
		idle_recent_1:	begin
						ready_0 = 0;
						ready_1 = 0;
						data_out=data_in_1;
						out_FIFO_wr=0;
						end

		msg_0:			begin
						ready_0=~out_FIFO_full;
						ready_1=0;
						data_out=data_in_0;
						out_FIFO_wr=~out_FIFO_full && req_0;
						end
						
		msg_1:			begin
						ready_0=0;
						ready_1=~out_FIFO_full;
						data_out=data_in_1;
						out_FIFO_wr=~out_FIFO_full && req_1;
						end
	endcase
end

endmodule
						
						
						
						
	

