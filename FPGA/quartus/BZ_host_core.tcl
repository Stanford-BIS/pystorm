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
# File: BZ_host_core.tcl
# Generated on: Wed Nov 29 17:25:39 2017

# Load Quartus Prime Tcl Project package
package require ::quartus::project

set need_to_close_project 0
set make_assignments 1

# Check that the right project is open
if {[is_project_open]} {
	if {[string compare $quartus(project) "BZ_host_core"]} {
		puts "Project BZ_host_core is not open"
		set make_assignments 0
	}
} else {
	# Only open if not already open
	if {[project_exists BZ_host_core]} {
		project_open -revision BZ_host_core BZ_host_core
	} else {
		project_new -revision BZ_host_core BZ_host_core
	}
	set need_to_close_project 1
}

# Make assignments
if {$make_assignments} {
	set_global_assignment -name FAMILY "Cyclone V"
	set_global_assignment -name DEVICE 5CEFA2U19C8
	set_global_assignment -name ORIGINAL_QUARTUS_VERSION 17.0.0
	set_global_assignment -name PROJECT_CREATION_TIME_DATE "16:30:57  NOVEMBER 28, 2017"
	set_global_assignment -name LAST_QUARTUS_VERSION "17.0.0 Lite Edition"
	set_global_assignment -name PROJECT_OUTPUT_DIRECTORY output_files
	set_global_assignment -name MIN_CORE_JUNCTION_TEMP 0
	set_global_assignment -name MAX_CORE_JUNCTION_TEMP 85
	set_global_assignment -name DEVICE_FILTER_PACKAGE UFBGA
	set_global_assignment -name DEVICE_FILTER_PIN_COUNT 169
	set_global_assignment -name ERROR_CHECK_FREQUENCY_DIVISOR 256
	set_global_assignment -name SYSTEMVERILOG_FILE ../src/router/BZ_host_core.sv
	set_global_assignment -name SYSTEMVERILOG_FILE ../src/router/stack_core.sv
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
	set_global_assignment -name QIP_FILE SysClkBuf.qip
	set_global_assignment -name VERILOG_FILE okLibrary.v
	set_global_assignment -name VERILOG_FILE okHost.v
	set_global_assignment -name SYSTEMVERILOG_FILE okHost.sv
	set_global_assignment -name VERILOG_FILE okEndpoints.v
	set_global_assignment -name SYSTEMVERILOG_FILE ../src/OKCoreBD.sv
	set_global_assignment -name SDC_FILE sys_clk.sdc
	set_global_assignment -name SDC_FILE okHost.sdc
	set_global_assignment -name QIP_FILE BZ_host_core_PLL.qip
	set_global_assignment -name SIP_FILE BZ_host_core_PLL.sip
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

	# Including default assignments
	set_global_assignment -name REVISION_TYPE BASE -family "Cyclone V"
	set_global_assignment -name TIMEQUEST_MULTICORNER_ANALYSIS ON -family "Cyclone V"
	set_global_assignment -name TIMEQUEST_REPORT_WORST_CASE_TIMING_PATHS OFF -family "Cyclone V"
	set_global_assignment -name TIMEQUEST_CCPP_TRADEOFF_TOLERANCE 0 -family "Cyclone V"
	set_global_assignment -name TDC_CCPP_TRADEOFF_TOLERANCE 30 -family "Cyclone V"
	set_global_assignment -name TIMEQUEST_DO_CCPP_REMOVAL ON -family "Cyclone V"
	set_global_assignment -name TIMEQUEST_SPECTRA_Q OFF -family "Cyclone V"
	set_global_assignment -name SYNTH_TIMING_DRIVEN_SYNTHESIS ON -family "Cyclone V"
	set_global_assignment -name SYNCHRONIZATION_REGISTER_CHAIN_LENGTH 3 -family "Cyclone V"
	set_global_assignment -name SYNTH_RESOURCE_AWARE_INFERENCE_FOR_BLOCK_RAM ON -family "Cyclone V"
	set_global_assignment -name OPTIMIZE_HOLD_TIMING "ALL PATHS" -family "Cyclone V"
	set_global_assignment -name OPTIMIZE_MULTI_CORNER_TIMING ON -family "Cyclone V"
	set_global_assignment -name AUTO_DELAY_CHAINS ON -family "Cyclone V"
	set_global_assignment -name ACTIVE_SERIAL_CLOCK FREQ_100MHZ -family "Cyclone V"
	set_global_assignment -name ADVANCED_PHYSICAL_OPTIMIZATION ON -family "Cyclone V"

	# Commit assignments
	export_assignments

	# Close project
	if {$need_to_close_project} {
		project_close
	}
}
