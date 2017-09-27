# Copyright (C) 2017  Intel Corporation. All rights reserved.
# Your use of Intel Corporation's design tools, logic functions 
# and other software and tools, and its AMPP partner logic 
# functions, and any output files from any of the foregoing 
# (including device programming or simulation files), and any 
# associated documentation or information are expressly subject 
# to the terms and conditions of the Intel Program License 
# Subscription Agreement, the Intel Quartus Prime License Agreement,
# the Intel MegaCore Function License Agreement, or other 
# applicable license agreement, including, without limitation, 
# that your use is for the sole purpose of programming logic 
# devices manufactured by Intel and sold by Intel or its 
# authorized distributors.  Please refer to the applicable 
# agreement for further details.

# Quartus Prime: Generate Tcl File for Project
# File: OKCoreBD.tcl
# Generated on: Tue Sep 26 16:46:34 2017

# Load Quartus Prime Tcl Project package
package require ::quartus::project

set need_to_close_project 0
set make_assignments 1

# Check that the right project is open
if {[is_project_open]} {
	if {[string compare $quartus(project) "OKCoreBD"]} {
		puts "Project OKCoreBD is not open"
		set make_assignments 0
	}
} else {
	# Only open if not already open
	if {[project_exists OKCoreBD]} {
		project_open -revision OKCoreBD OKCoreBD
	} else {
		project_new -revision OKCoreBD OKCoreBD
	}
	set need_to_close_project 1
}

# Make assignments
if {$make_assignments} {
	set_global_assignment -name FAMILY "Cyclone V"
	set_global_assignment -name DEVICE 5CEFA2U19C8
	set_global_assignment -name PROJECT_OUTPUT_DIRECTORY output_files
	set_global_assignment -name ERROR_CHECK_FREQUENCY_DIVISOR 256
	set_global_assignment -name MIN_CORE_JUNCTION_TEMP 0
	set_global_assignment -name MAX_CORE_JUNCTION_TEMP 85
	set_global_assignment -name ENABLE_OCT_DONE OFF
	set_global_assignment -name STRATIXV_CONFIGURATION_SCHEME "PASSIVE PARALLEL X16"
	set_global_assignment -name USE_CONFIGURATION_DEVICE OFF
	set_global_assignment -name GENERATE_RBF_FILE ON
	set_global_assignment -name CRC_ERROR_OPEN_DRAIN ON
	set_global_assignment -name RESERVE_DATA15_THROUGH_DATA8_AFTER_CONFIGURATION "USE AS REGULAR IO"
	set_global_assignment -name RESERVE_DATA7_THROUGH_DATA5_AFTER_CONFIGURATION "USE AS REGULAR IO"
	set_global_assignment -name OUTPUT_IO_TIMING_NEAR_END_VMEAS "HALF VCCIO" -rise
	set_global_assignment -name OUTPUT_IO_TIMING_NEAR_END_VMEAS "HALF VCCIO" -fall
	set_global_assignment -name OUTPUT_IO_TIMING_FAR_END_VMEAS "HALF SIGNAL SWING" -rise
	set_global_assignment -name OUTPUT_IO_TIMING_FAR_END_VMEAS "HALF SIGNAL SWING" -fall
	set_global_assignment -name POWER_PRESET_COOLING_SOLUTION "23 MM HEAT SINK WITH 200 LFPM AIRFLOW"
	set_global_assignment -name POWER_BOARD_THERMAL_MODEL "NONE (CONSERVATIVE)"
	set_global_assignment -name IOBANK_VCCIO 1.8V -section_id 2A
	set_global_assignment -name IOBANK_VCCIO 1.8V -section_id 5B
	set_global_assignment -name IOBANK_VCCIO 1.8V -section_id 5A
	set_global_assignment -name IOBANK_VCCIO 1.8V -section_id 4A
	set_global_assignment -name PARTITION_NETLIST_TYPE SOURCE -section_id Top
	set_global_assignment -name PARTITION_FITTER_PRESERVATION_LEVEL PLACEMENT_AND_ROUTING -section_id Top
	set_global_assignment -name PARTITION_COLOR 16764057 -section_id Top
	set_global_assignment -name QIP_FILE BDIOPLL.qip
	set_global_assignment -name SIP_FILE BDIOPLL.sip
	set_global_assignment -name QIP_FILE SysClkBuf.qip
	set_global_assignment -name QIP_FILE SpikeGeneratorMem.qip
	set_global_assignment -name QIP_FILE SpikeFilterMem.qip
	set_global_assignment -name QIP_FILE PipeOutFIFO.qip
	set_global_assignment -name QIP_FILE PipeInFIFO.qip
	set_global_assignment -name VERILOG_FILE okLibrary.v
	set_global_assignment -name VERILOG_FILE okHost.v
	set_global_assignment -name SYSTEMVERILOG_FILE okHost.sv
	set_global_assignment -name VERILOG_FILE okEndpoints.v
	set_global_assignment -name QIP_FILE BDOutFIFO.qip
	set_global_assignment -name QIP_FILE BDInFIFO.qip
	set_global_assignment -name SYSTEMVERILOG_FILE ../src/OKCoreBD.sv
	set_global_assignment -name SDC_FILE sys_clk.sdc
	set_global_assignment -name SDC_FILE okHost.sdc
	set_location_assignment PIN_W12 -to pReset
	set_location_assignment PIN_AA15 -to sReset
	set_location_assignment PIN_AB17 -to BD_in_clk
	set_location_assignment PIN_AB15 -to BD_in_ready
	set_location_assignment PIN_T13 -to _BD_in_valid
	set_location_assignment PIN_U13 -to BD_in_data[33]
	set_location_assignment PIN_R12 -to BD_in_data[32]
	set_location_assignment PIN_AB16 -to BD_in_data[31]
	set_location_assignment PIN_W14 -to BD_in_data[30]
	set_location_assignment PIN_Y14 -to BD_in_data[29]
	set_location_assignment PIN_AA18 -to BD_in_data[28]
	set_location_assignment PIN_V13 -to BD_in_data[27]
	set_location_assignment PIN_W13 -to BD_in_data[26]
	set_location_assignment PIN_T12 -to BD_in_data[25]
	set_location_assignment PIN_U12 -to BD_in_data[24]
	set_location_assignment PIN_R15 -to BD_in_data[23]
	set_location_assignment PIN_T15 -to BD_in_data[22]
	set_location_assignment PIN_Y15 -to BD_in_data[21]
	set_location_assignment PIN_AB18 -to BD_in_data[20]
	set_location_assignment PIN_P14 -to BD_in_data[19]
	set_location_assignment PIN_V15 -to BD_in_data[18]
	set_location_assignment PIN_R14 -to BD_in_data[17]
	set_location_assignment PIN_Y20 -to BD_in_data[16]
	set_location_assignment PIN_AA20 -to BD_in_data[15]
	set_location_assignment PIN_Y19 -to BD_in_data[14]
	set_location_assignment PIN_AA19 -to BD_in_data[13]
	set_location_assignment PIN_AB21 -to BD_in_data[12]
	set_location_assignment PIN_AB20 -to BD_in_data[11]
	set_location_assignment PIN_Y16 -to BD_in_data[10]
	set_location_assignment PIN_U17 -to BD_in_data[9]
	set_location_assignment PIN_W17 -to BD_in_data[8]
	set_location_assignment PIN_Y22 -to BD_in_data[7]
	set_location_assignment PIN_AB22 -to BD_in_data[6]
	set_location_assignment PIN_Y17 -to BD_in_data[5]
	set_location_assignment PIN_U15 -to BD_in_data[4]
	set_location_assignment PIN_AA22 -to BD_in_data[3]
	set_location_assignment PIN_W22 -to BD_in_data[2]
	set_location_assignment PIN_W21 -to BD_in_data[1]
	set_location_assignment PIN_Y21 -to BD_in_data[0]
	set_location_assignment PIN_G18 -to BD_out_clk
	set_location_assignment PIN_V20 -to BD_out_ready
	set_location_assignment PIN_V19 -to BD_out_valid
	set_location_assignment PIN_N1 -to BD_out_data[0]
	set_location_assignment PIN_L1 -to BD_out_data[1]
	set_location_assignment PIN_W2 -to BD_out_data[2]
	set_location_assignment PIN_L2 -to BD_out_data[3]
	set_location_assignment PIN_N2 -to BD_out_data[4]
	set_location_assignment PIN_U2 -to BD_out_data[5]
	set_location_assignment PIN_U1 -to BD_out_data[6]
	set_location_assignment PIN_K16 -to BD_out_data[7]
	set_location_assignment PIN_J16 -to BD_out_data[8]
	set_location_assignment PIN_J18 -to BD_out_data[9]
	set_location_assignment PIN_G17 -to BD_out_data[10]
	set_location_assignment PIN_H20 -to BD_out_data[11]
	set_location_assignment PIN_T19 -to BD_out_data[12]
	set_location_assignment PIN_E20 -to BD_out_data[13]
	set_location_assignment PIN_H19 -to BD_out_data[14]
	set_location_assignment PIN_R16 -to BD_out_data[15]
	set_location_assignment PIN_R17 -to BD_out_data[16]
	set_location_assignment PIN_R19 -to BD_out_data[17]
	set_location_assignment PIN_P19 -to BD_out_data[18]
	set_location_assignment PIN_T17 -to BD_out_data[19]
	set_location_assignment PIN_T18 -to BD_out_data[20]
	set_location_assignment PIN_J17 -to adc0
	set_location_assignment PIN_F20 -to adc1
	set_instance_assignment -name IO_STANDARD "1.8 V" -to pReset
	set_instance_assignment -name IO_STANDARD "1.8 V" -to sReset
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_clk
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_ready
	set_instance_assignment -name IO_STANDARD "1.8 V" -to _BD_in_valid
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[*]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_clk
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_ready
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_valid
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[*]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to adc0
	set_instance_assignment -name IO_STANDARD "1.8 V" -to adc1
	set_location_assignment PIN_P9 -to okUH[0]
	set_location_assignment PIN_M10 -to okUH[1]
	set_location_assignment PIN_L9 -to okUH[2]
	set_location_assignment PIN_Y11 -to okUH[3]
	set_location_assignment PIN_W11 -to okUH[4]
	set_location_assignment PIN_P12 -to okHU[0]
	set_location_assignment PIN_R11 -to okHU[1]
	set_location_assignment PIN_AB11 -to okHU[2]
	set_location_assignment PIN_AA7 -to okUHU[0]
	set_location_assignment PIN_W7 -to okUHU[1]
	set_location_assignment PIN_T7 -to okUHU[2]
	set_location_assignment PIN_Y7 -to okUHU[3]
	set_location_assignment PIN_AB7 -to okUHU[4]
	set_location_assignment PIN_U7 -to okUHU[5]
	set_location_assignment PIN_P6 -to okUHU[6]
	set_location_assignment PIN_U6 -to okUHU[7]
	set_location_assignment PIN_N6 -to okUHU[8]
	set_location_assignment PIN_R5 -to okUHU[9]
	set_location_assignment PIN_M6 -to okUHU[10]
	set_location_assignment PIN_R6 -to okUHU[11]
	set_location_assignment PIN_M7 -to okUHU[12]
	set_location_assignment PIN_L7 -to okUHU[13]
	set_location_assignment PIN_R7 -to okUHU[14]
	set_location_assignment PIN_L8 -to okUHU[15]
	set_location_assignment PIN_W8 -to okUHU[16]
	set_location_assignment PIN_V8 -to okUHU[17]
	set_location_assignment PIN_M8 -to okUHU[18]
	set_location_assignment PIN_N8 -to okUHU[19]
	set_location_assignment PIN_N10 -to okUHU[20]
	set_location_assignment PIN_N9 -to okUHU[21]
	set_location_assignment PIN_R10 -to okUHU[22]
	set_location_assignment PIN_AA10 -to okUHU[23]
	set_location_assignment PIN_Y9 -to okUHU[24]
	set_location_assignment PIN_R9 -to okUHU[25]
	set_location_assignment PIN_V9 -to okUHU[26]
	set_location_assignment PIN_U8 -to okUHU[27]
	set_location_assignment PIN_AA8 -to okUHU[28]
	set_location_assignment PIN_AB8 -to okUHU[29]
	set_location_assignment PIN_T9 -to okUHU[30]
	set_location_assignment PIN_T8 -to okUHU[31]
	set_location_assignment PIN_T10 -to okAA
	set_location_assignment PIN_U10 -to user_reset
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUH[0]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUH[1]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUH[2]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUH[3]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUH[4]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okHU[0]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okHU[1]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okHU[2]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[0]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[1]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[2]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[3]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[4]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[5]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[6]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[7]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[8]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[9]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[10]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[11]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[12]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[13]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[14]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[15]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[16]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[17]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[18]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[19]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[20]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[21]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[22]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[23]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[24]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[25]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[26]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[27]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[28]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[29]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[30]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okUHU[31]
	set_instance_assignment -name FAST_INPUT_REGISTER ON -to okUH[1]
	set_instance_assignment -name FAST_INPUT_REGISTER ON -to okUH[2]
	set_instance_assignment -name FAST_INPUT_REGISTER ON -to okUH[3]
	set_instance_assignment -name FAST_INPUT_REGISTER ON -to okUH[4]
	set_instance_assignment -name FAST_INPUT_REGISTER ON -to okUHU[*]
	set_instance_assignment -name FAST_OUTPUT_REGISTER ON -to okUHU[*]
	set_instance_assignment -name MATCH_PLL_COMPENSATION_CLOCK ON -to "OKIfc:ok_ifc|okHost:okHI|ok_altera_pll:ok_altera_pll0|altera_pll:altera_pll_i|outclk_wire"
	set_instance_assignment -name PLL_COMPENSATION_MODE "SOURCE SYNCHRONOUS" -to "OKIfc:ok_ifc|okHost:okHI|ok_altera_pll:ok_altera_pll0|altera_pll:altera_pll_i|*"
	set_instance_assignment -name PLL_AUTO_RESET ON -to "OKIfc:ok_ifc|okHost:okHI|ok_altera_pll:ok_altera_pll0|altera_pll:altera_pll_i|*"
	set_instance_assignment -name PLL_BANDWIDTH_PRESET AUTO -to "OKIfc:ok_ifc|okHost:okHI|ok_altera_pll:ok_altera_pll0|altera_pll:altera_pll_i|*"
	set_instance_assignment -name D1_DELAY 5 -to okUHU[14]
	set_instance_assignment -name D1_DELAY 5 -to okUHU[6]
	set_instance_assignment -name D1_DELAY 5 -to okUHU[10]
	set_instance_assignment -name D1_DELAY 5 -to okUHU[27]
	set_instance_assignment -name D1_DELAY 5 -to okUHU[25]
	set_instance_assignment -name D1_DELAY 5 -to okUHU[31]
	set_instance_assignment -name D1_DELAY 5 -to okUHU[21]
	set_instance_assignment -name D1_DELAY 5 -to okUHU[19]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to okAA
	set_instance_assignment -name IO_STANDARD "2.5 V" -to user_reset
	set_location_assignment PIN_H10 -to sys_clk_p
	set_instance_assignment -name IO_STANDARD LVDS -to sys_clk_p
	set_location_assignment PIN_G11 -to sys_clk_n
	set_instance_assignment -name IO_STANDARD LVDS -to sys_clk_n
	set_location_assignment PIN_A20 -to led[0]
	set_location_assignment PIN_A22 -to led[1]
	set_location_assignment PIN_B20 -to led[2]
	set_location_assignment PIN_C20 -to led[3]
	set_instance_assignment -name IO_STANDARD "1.5 V" -to led[3]
	set_instance_assignment -name IO_STANDARD "1.5 V" -to led[2]
	set_instance_assignment -name IO_STANDARD "1.5 V" -to led[1]
	set_instance_assignment -name IO_STANDARD "1.5 V" -to led[0]
	set_instance_assignment -name PARTITION_HIERARCHY root_partition -to | -section_id Top

	# Commit assignments
	export_assignments

	# Close project
	if {$need_to_close_project} {
		project_close
	}
}
