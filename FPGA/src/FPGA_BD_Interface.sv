`define NUM_BITS_PIN2CORE 21
`define NUM_BITS_CORE2PIN 34

module FPGA_TO_BD ( _Reset, clk, req, x, xe);
        parameter ONEOF=`NUM_BITS_PIN2CORE;
        parameter FILENAME={`TBPATH, "p2c_in.dat"}; //file.txt
        parameter NAME = "SOURCE_P2C_CLOCKED";
        parameter DELAY = 1;
        parameter CONST = -1;

        input _Reset, clk, xe;
        output req;
        output x;
        reg req;
        reg[1:0] read_number;
        reg [(ONEOF)-1:0]x;
        integer fd,  error;
        reg[(ONEOF)-1:0] value[ 0:`MAX_INPUTS-1];
        reg[1000:0] s;
        integer i;
        integer nvalues;

        initial begin
             if (CONST == -1) begin
                fd = $fopen(FILENAME,"r");
                if (fd == 0) begin
                        $display ("%s: Couldn't open data file for inject_source_clocked: %s", NAME, FILENAME);
                        $finish;
                end
                else begin
                        $display("%s, Data File opened correctly", NAME);
                        error = $fscanf(fd, "%d", nvalues);
                        $display("%s, Reading %d values from file %s to source", NAME, nvalues, FILENAME);
                end
                i = 0;
                while( i < nvalues) begin
                        error = $fgets(s,fd);
                        error = $fscanf(fd, "%d", value[i]);
                        $display("Reading value %d", value[i]);
                        if ( value[i] >=  2**(ONEOF) ) begin
                                $display("This value is greater than expected: %s = %d", NAME, value[i]);
                                $finish;
                        end
                        i=i+1;
                end
              end
              else begin
                if ( CONST >=  2**(ONEOF) ) begin  // ???
                            $display("This value is greater than expected: %s = %d", NAME, CONST);
                            $finish;
                end
              end
              i = 0;
              x <= #DELAY 0;
              req <= #DELAY 0;
        end

        always @(posedge clk or negedge _Reset) begin

                if(xe && _Reset && ~req) begin
                    if (CONST == -1) begin
                        $display("%s, READY TO SEND - new value at time %t", NAME, $time);
                        if ( i < nvalues) begin
                                $display("%s, Sourcing %d value from file= %d", NAME, i, value[i]);
                                x <= #DELAY value[i]; // (1<<value[i]);
                                i <= #DELAY i+1;
                                $display("Req goes high \n");
				req <= #(DELAY+1) 1'b1;
                        end
                        else  begin
                                 $display("%s: Already sourced all values", NAME);
                        end
                    end
                    else begin
                        $display ("%s, Sourcing %d", NAME, CONST);
                        x <= #DELAY (1 << CONST);
                        i <= #DELAY i+1;
                    end
                end
                else if(~_Reset || ~xe)
                begin
                        $display("%s: Source/Req  is in Reset Phase", NAME);
                        //x <= #DELAY 0;
			//req <= #DELAY 0;
                end
        end

        always @(negedge clk) begin
                if (req == 1 && xe == 0) begin

		req <= #DELAY 0;

                end
        end

        always @( x ) begin
                $display("%s, Data channels set to %b", NAME, x );
        end
endmodule

module BD_TO_FPGA (_Reset, x, Ready, Valid, clk);
        parameter NUM_BITS=`NUM_BITS_CORE2PIN;
        parameter NAME="SINK_C2P_CLOCKED";
        parameter DELAY = 1;
        parameter FILENAME = {`TBPATH, "c2p_out.dat"}; //output.dat
        input _Reset, Valid;

        input clk;
        input  x;
        output Ready;
        wire[0: (NUM_BITS)-1] x;
        reg Ready;
        integer file;

        initial begin
                //x <= #DELAY 0;
                Ready <= 1;
                //seed = SEED;
                file = $fopen(FILENAME, "w");
                if (file == 0) begin
                        $finish;
                end else begin
                        $display("%s, Output Data File opened correctly", NAME);
                end

        end
        always @(negedge clk) begin
                //if ((clk==0) && (Valid==1)  && (_Reset==1) && (Ready==1)) begin
                if ((clk==0) && (Valid==0)  && (_Reset==1) && (Ready==1)) begin

                        $fwrite(file, x);
                        $fwrite(file, "\n");
                        $display(" writing %d to the output file", x);
                        Ready <= #DELAY 0;
                end

                //else if ((clk==0) && (Valid==0) && (Ready==1) &&  (_Reset==1))
                else if ((clk==0) && (Valid==0) && (Ready==0) &&  _Reset)
                begin
                        $display("No data is recieved  at this cycle\n");
                end

        end

        always @(~_Reset) begin
                if (~_Reset)
                begin
                   Ready <= #DELAY 1;
                end

        end

        always @(posedge clk) begin
                if ( (Valid ==1) && (Ready ==0)) begin
                //if ( (Valid == 0) && (Ready == 0)) begin
                        //$fwrite(file, x);
                       // $fwrite(file, "\n");

			Ready <= #DELAY 1;


                end
        end

        always @( x) begin
                  $display("%s. Data =%b, Ready=%d Valid=%d at time %t\n", NAME,  x,  Ready, Valid, $time);

        end
endmodule