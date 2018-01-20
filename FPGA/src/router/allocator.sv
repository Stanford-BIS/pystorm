//output Allocator

//In each router node, there is one allocator per output.
//Its function is to allocate the output to one of two possible inputs.
//If both inputs simultaneously request the same output, the allocator
//forces the input which has most recently been granted the output to wait,
//while allocating the output to the other input.

//The top output can be allocated to either the bottom or Braindrop input.
//The Braindrop output can be allocated to either the top or bottom input.
//The bottom output can be allocated to either the the top or Braindrop input.
//In other words, a packet must either continue in its original direction or be diverted to the braindrop. It cannot turn around.
module allocator (
input clk,
input reset,
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


//4 states
reg [1:0] state = 2'b00;  
reg [1:0] next_state;

parameter idle_recent_0 = 2'b00; 
parameter msg_0 = 2'b01;  
parameter msg_1 = 2'b10; 
parameter idle_recent_1 = 2'b11;


//State Register
always @ (posedge clk or posedge reset)
	begin: state_register
	if(reset == 1)
		state <= idle_recent_0;
	else
		state <= next_state;
end

//Next State combinational logic
always @ (state or tail_0 or tail_1 or req_0 or req_1)
begin : next_state_logic
	next_state = state;
	case(state)
		//Idle. Has most recently allocated the output to input 0.
		idle_recent_0:	begin
						if(req_1==1)
							//input 1 has priority in this state
							next_state=msg_1;
						else if(req_0==1)
							next_state=msg_0;
						end
		//Idle. Has most recently allocated the output to input 1.				
		idle_recent_1:	begin
						if(req_0==1)
							//input 0 has priority in this state
							next_state=msg_0;
						else if(req_1==1)
							next_state=msg_1;
						end
		
		//The output has been allocated to input 0.
		msg_0:			begin
						if((tail_0==1) && (req_1==1))
							next_state=msg_1;
						else if(tail_0==1)
							next_state=idle_recent_0;
						end
		
		//The output has been allocated to input 1.
		msg_1:			begin
						if((tail_1==1) && (req_0==1))
							next_state=msg_0;
						else if(tail_1==1)
							next_state=idle_recent_1;
						end
		
		default:		next_state=state;
	
	endcase
end



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
						ready_0=~out_FIFO_full; //ready for input 0 as long as the output FIFO is not full
						ready_1=0;
						data_out=data_in_0;
						out_FIFO_wr=~out_FIFO_full && req_0; //write input 0 to the output fifo as long as it is not full, and input 0 is valid
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