onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -divider OKIfc
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/okUH
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/okHU
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/okUHU
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/okAA
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/led
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/okClk
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/user_reset
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/okHE
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/okEH
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/pipe_in_write
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/pipe_in_ready
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/pipe_in_data
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/pipe_out_read
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/pipe_out_ready
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/pipe_out_data
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/okEHx
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/FIFO_in_count
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/FIFO_in_data_out
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/FIFO_in_rd_ack
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/FIFO_in_empty
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/FIFO_in_full
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/FIFO_out_count
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/FIFO_out_data_in
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/FIFO_out_data_out
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/FIFO_out_rd_ack
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/FIFO_out_wr
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/FIFO_out_empty
add wave -noupdate -divider {OKIfc->Core Channels}
add wave -noupdate /OKCoreBD_tb/dut/ok_ifc/FIFO_out_full
add wave -noupdate /OKCoreBD_tb/dut/PC_downstream/d
add wave -noupdate /OKCoreBD_tb/dut/PC_downstream/v
add wave -noupdate /OKCoreBD_tb/dut/PC_downstream/a
add wave -noupdate /OKCoreBD_tb/dut/PC_upstream/d
add wave -noupdate /OKCoreBD_tb/dut/PC_upstream/v
add wave -noupdate -divider Core
add wave -noupdate /OKCoreBD_tb/dut/PC_upstream/a
add wave -noupdate /OKCoreBD_tb/dut/core/pReset
add wave -noupdate /OKCoreBD_tb/dut/core/sReset
add wave -noupdate /OKCoreBD_tb/dut/core/adc0
add wave -noupdate /OKCoreBD_tb/dut/core/adc1
add wave -noupdate /OKCoreBD_tb/dut/core/clk
add wave -noupdate /OKCoreBD_tb/dut/core/reset
add wave -noupdate /OKCoreBD_tb/dut/core/conf_regs
add wave -noupdate /OKCoreBD_tb/dut/core/conf_reg_reset_vals
add wave -noupdate /OKCoreBD_tb/dut/core/time_unit_pulse
add wave -noupdate /OKCoreBD_tb/dut/core/send_HB_up_pulse
add wave -noupdate /OKCoreBD_tb/dut/core/stall_dn
add wave -noupdate -divider {Core->BDIfc Channels}
add wave -noupdate /OKCoreBD_tb/dut/core/time_elapsed
add wave -noupdate /OKCoreBD_tb/dut/BD_downstream/d
add wave -noupdate /OKCoreBD_tb/dut/BD_downstream/v
add wave -noupdate /OKCoreBD_tb/dut/BD_downstream/a
add wave -noupdate /OKCoreBD_tb/dut/BD_upstream/d
add wave -noupdate /OKCoreBD_tb/dut/BD_upstream/v
add wave -noupdate /OKCoreBD_tb/dut/BD_upstream/a
add wave -noupdate -divider BDIfc
add wave -noupdate /OKCoreBD_tb/dut/BD_in_clk_int
add wave -noupdate /OKCoreBD_tb/dut/BD_in_clk
add wave -noupdate /OKCoreBD_tb/dut/BD_out_clk_int
add wave -noupdate /OKCoreBD_tb/dut/BD_out_clk
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/BD_out_ready
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/BD_out_valid
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/BD_out_data
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/BD_in_ready
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/BD_in_valid
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/BD_in_data
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/core_clk
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/BD_out_clk_int
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/BD_in_clk_int
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/reset
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/out_FIFO_data_in
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/out_FIFO_wr_en
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/out_FIFO_wr_full
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/out_FIFO_wr_clk
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/out_FIFO_data_out
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/out_FIFO_rd_ack
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/out_FIFO_rd_empty
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/out_FIFO_rd_clk
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/in_FIFO_data_in
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/in_FIFO_wr_en
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/in_FIFO_wr_full
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/in_FIFO_wr_clk
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/in_FIFO_data_out
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/in_FIFO_rd_ack
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/in_FIFO_rd_empty
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/in_FIFO_rd_clk
add wave -noupdate /OKCoreBD_tb/dut/BD_ifc/user_reset
add wave -noupdate /OKCoreBD_tb/dut/sys_clk_buf/datain
add wave -noupdate /OKCoreBD_tb/dut/sys_clk_buf/datain_b
add wave -noupdate /OKCoreBD_tb/dut/sys_clk_buf/dataout
add wave -noupdate /OKCoreBD_tb/dut/sys_clk_buf/sub_wire0
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {110581 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 318
configure wave -valuecolwidth 100
configure wave -justifyvalue left
configure wave -signalnamewidth 0
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ps
update
WaveRestoreZoom {0 ps} {494918 ps}
