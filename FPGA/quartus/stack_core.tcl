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
# File: stack_core.tcl
# Generated on: Fri Dec 08 16:00:17 2017

# Load Quartus Prime Tcl Project package
package require ::quartus::project

set need_to_close_project 0
set make_assignments 1

# Check that the right project is open
if {[is_project_open]} {
	if {[string compare $quartus(project) "stack_core"]} {
		puts "Project stack_core is not open"
		set make_assignments 0
	}
} else {
	# Only open if not already open
	if {[project_exists stack_core]} {
		project_open -revision stack_core stack_core
	} else {
		project_new -revision stack_core stack_core
	}
	set need_to_close_project 1
}

# Make assignments
if {$make_assignments} {
	set_global_assignment -name FAMILY "MAX 10"
	set_global_assignment -name DEVICE 10M08SAU169C8G
	set_global_assignment -name ORIGINAL_QUARTUS_VERSION 17.0.0
	set_global_assignment -name PROJECT_CREATION_TIME_DATE "15:10:37  NOVEMBER 28, 2017"
	set_global_assignment -name LAST_QUARTUS_VERSION "17.0.0 Lite Edition"
	set_global_assignment -name PROJECT_OUTPUT_DIRECTORY output_files
	set_global_assignment -name MIN_CORE_JUNCTION_TEMP 0
	set_global_assignment -name MAX_CORE_JUNCTION_TEMP 85
	set_global_assignment -name DEVICE_FILTER_PACKAGE UFBGA
	set_global_assignment -name DEVICE_FILTER_PIN_COUNT 169
	set_global_assignment -name ERROR_CHECK_FREQUENCY_DIVISOR 256
	set_global_assignment -name POWER_PRESET_COOLING_SOLUTION "23 MM HEAT SINK WITH 200 LFPM AIRFLOW"
	set_global_assignment -name POWER_BOARD_THERMAL_MODEL "NONE (CONSERVATIVE)"
	set_global_assignment -name FLOW_ENABLE_POWER_ANALYZER ON
	set_global_assignment -name POWER_DEFAULT_INPUT_IO_TOGGLE_RATE "12.5 %"
	set_global_assignment -name SYSTEMVERILOG_FILE ../src/router/stack_core.sv
	set_global_assignment -name QIP_FILE stack_BDIO_PLL.qip
	set_global_assignment -name PARTITION_NETLIST_TYPE SOURCE -section_id Top
	set_global_assignment -name PARTITION_FITTER_PRESERVATION_LEVEL PLACEMENT_AND_ROUTING -section_id Top
	set_global_assignment -name PARTITION_COLOR 16764057 -section_id Top
	set_global_assignment -name ENABLE_OCT_DONE OFF
	set_global_assignment -name ENABLE_BOOT_SEL_PIN OFF
	set_global_assignment -name STRATIXV_CONFIGURATION_SCHEME "PASSIVE SERIAL"
	set_global_assignment -name USE_CONFIGURATION_DEVICE ON
	set_global_assignment -name CRC_ERROR_OPEN_DRAIN OFF
	set_global_assignment -name OUTPUT_IO_TIMING_NEAR_END_VMEAS "HALF VCCIO" -rise
	set_global_assignment -name OUTPUT_IO_TIMING_NEAR_END_VMEAS "HALF VCCIO" -fall
	set_global_assignment -name OUTPUT_IO_TIMING_FAR_END_VMEAS "HALF SIGNAL SWING" -rise
	set_global_assignment -name OUTPUT_IO_TIMING_FAR_END_VMEAS "HALF SIGNAL SWING" -fall
	set_global_assignment -name IOBANK_VCCIO 1.8V -section_id 8
	set_global_assignment -name IOBANK_VCCIO 1.8V -section_id 6
	set_global_assignment -name IOBANK_VCCIO 1.8V -section_id 5
	set_global_assignment -name IOBANK_VCCIO 1.8V -section_id 3
	set_global_assignment -name IOBANK_VCCIO 1.8V -section_id 2
	set_global_assignment -name IOBANK_VCCIO 1.8V -section_id 1B
	set_global_assignment -name IOBANK_VCCIO 1.8V -section_id 1A
	set_location_assignment PIN_B2 -to top_out_clk
	set_location_assignment PIN_E12 -to top_out[0]
	set_location_assignment PIN_D12 -to top_out[1]
	set_location_assignment PIN_C12 -to top_out[2]
	set_location_assignment PIN_B12 -to top_out[3]
	set_location_assignment PIN_B11 -to top_out[4]
	set_location_assignment PIN_B10 -to top_out[5]
	set_location_assignment PIN_B9 -to top_out[6]
	set_location_assignment PIN_A8 -to top_out[7]
	set_location_assignment PIN_B7 -to top_out[8]
	set_location_assignment PIN_B6 -to top_out[9]
	set_location_assignment PIN_B5 -to top_out[10]
	set_location_assignment PIN_B4 -to top_valid_out
	set_location_assignment PIN_B3 -to top_ready_in
	set_location_assignment PIN_A2 -to top_ready_out
	set_location_assignment PIN_A3 -to top_valid_in
	set_location_assignment PIN_D13 -to top_in[0]
	set_location_assignment PIN_C13 -to top_in[1]
	set_location_assignment PIN_B13 -to top_in[2]
	set_location_assignment PIN_A12 -to top_in[3]
	set_location_assignment PIN_A11 -to top_in[4]
	set_location_assignment PIN_A10 -to top_in[5]
	set_location_assignment PIN_A9 -to top_in[6]
	set_location_assignment PIN_A7 -to top_in[7]
	set_location_assignment PIN_A6 -to top_in[8]
	set_location_assignment PIN_A5 -to top_in[9]
	set_location_assignment PIN_A4 -to top_in[10]
	set_location_assignment PIN_B1 -to top_in_clk
	set_location_assignment PIN_H13 -to bot_in[0]
	set_location_assignment PIN_H10 -to bot_in[1]
	set_location_assignment PIN_G10 -to bot_in[2]
	set_location_assignment PIN_F13 -to bot_in[3]
	set_location_assignment PIN_H8 -to bot_in[4]
	set_location_assignment PIN_F12 -to bot_in[5]
	set_location_assignment PIN_D11 -to bot_in[6]
	set_location_assignment PIN_E10 -to bot_in[7]
	set_location_assignment PIN_E9 -to bot_in[8]
	set_location_assignment PIN_D8 -to bot_in[9]
	set_location_assignment PIN_F8 -to bot_in[10]
	set_location_assignment PIN_C2 -to bot_in_clk
	set_location_assignment PIN_D6 -to bot_valid_in
	set_location_assignment PIN_E4 -to bot_ready_out
	set_location_assignment PIN_G13 -to bot_out[0]
	set_location_assignment PIN_G12 -to bot_out[1]
	set_location_assignment PIN_G9 -to bot_out[2]
	set_location_assignment PIN_E13 -to bot_out[3]
	set_location_assignment PIN_F10 -to bot_out[4]
	set_location_assignment PIN_F9 -to bot_out[5]
	set_location_assignment PIN_C11 -to bot_out[6]
	set_location_assignment PIN_C10 -to bot_out[7]
	set_location_assignment PIN_C9 -to bot_out[8]
	set_location_assignment PIN_D9 -to bot_out[9]
	set_location_assignment PIN_E8 -to bot_out[10]
	set_location_assignment PIN_D7 -to bot_valid_out
	set_location_assignment PIN_E6 -to bot_ready_in
	set_location_assignment PIN_F4 -to bot_out_clk
	set_location_assignment PIN_M8 -to pReset
	set_location_assignment PIN_J2 -to sReset
	set_location_assignment PIN_M11 -to BD_out_data[0]
	set_location_assignment PIN_M10 -to BD_out_data[1]
	set_location_assignment PIN_N11 -to BD_out_data[2]
	set_location_assignment PIN_N12 -to BD_out_data[3]
	set_location_assignment PIN_M12 -to BD_out_data[4]
	set_location_assignment PIN_M13 -to BD_out_data[5]
	set_location_assignment PIN_L13 -to BD_out_data[6]
	set_location_assignment PIN_K13 -to BD_out_data[7]
	set_location_assignment PIN_J13 -to BD_out_data[8]
	set_location_assignment PIN_K11 -to BD_out_data[9]
	set_location_assignment PIN_L12 -to BD_out_data[10]
	set_location_assignment PIN_K12 -to BD_out_data[11]
	set_location_assignment PIN_K8 -to BD_out_data[12]
	set_location_assignment PIN_L11 -to BD_out_data[13]
	set_location_assignment PIN_H9 -to BD_out_data[14]
	set_location_assignment PIN_K10 -to BD_out_data[15]
	set_location_assignment PIN_J10 -to BD_out_data[16]
	set_location_assignment PIN_J9 -to BD_out_data[17]
	set_location_assignment PIN_J8 -to BD_out_data[18]
	set_location_assignment PIN_K7 -to BD_out_data[19]
	set_location_assignment PIN_K6 -to BD_out_data[20]
	set_location_assignment PIN_C1 -to BD_out_clk_ifc
	set_location_assignment PIN_K5 -to BD_out_ready
	set_location_assignment PIN_L3 -to BD_out_valid
	set_location_assignment PIN_L4 -to BD_in_data[0]
	set_location_assignment PIN_J7 -to BD_in_data[1]
	set_location_assignment PIN_L5 -to BD_in_data[2]
	set_location_assignment PIN_J6 -to BD_in_data[3]
	set_location_assignment PIN_E3 -to BD_in_data[4]
	set_location_assignment PIN_G5 -to BD_in_data[5]
	set_location_assignment PIN_K2 -to BD_in_data[6]
	set_location_assignment PIN_J5 -to BD_in_data[7]
	set_location_assignment PIN_H5 -to BD_in_data[8]
	set_location_assignment PIN_H3 -to BD_in_data[9]
	set_location_assignment PIN_H4 -to BD_in_data[10]
	set_location_assignment PIN_G4 -to BD_in_data[11]
	set_location_assignment PIN_D1 -to BD_in_data[12]
	set_location_assignment PIN_E1 -to BD_in_data[13]
	set_location_assignment PIN_H1 -to BD_in_data[14]
	set_location_assignment PIN_J1 -to BD_in_data[15]
	set_location_assignment PIN_K1 -to BD_in_data[16]
	set_location_assignment PIN_L2 -to BD_in_data[17]
	set_location_assignment PIN_F1 -to BD_in_data[18]
	set_location_assignment PIN_H2 -to BD_in_data[19]
	set_location_assignment PIN_M4 -to BD_in_data[20]
	set_location_assignment PIN_M3 -to BD_in_data[21]
	set_location_assignment PIN_M2 -to BD_in_data[22]
	set_location_assignment PIN_L1 -to BD_in_data[23]
	set_location_assignment PIN_M1 -to BD_in_data[24]
	set_location_assignment PIN_N2 -to BD_in_data[25]
	set_location_assignment PIN_N3 -to BD_in_data[26]
	set_location_assignment PIN_N4 -to BD_in_data[27]
	set_location_assignment PIN_N5 -to BD_in_data[28]
	set_location_assignment PIN_N6 -to BD_in_data[29]
	set_location_assignment PIN_N7 -to BD_in_data[30]
	set_location_assignment PIN_N8 -to BD_in_data[31]
	set_location_assignment PIN_N9 -to BD_in_data[32]
	set_location_assignment PIN_N10 -to BD_in_data[33]
	set_location_assignment PIN_M5 -to BD_in_clk_ifc
	set_location_assignment PIN_M7 -to BD_in_ready
	set_location_assignment PIN_M9 -to _BD_in_valid
	set_location_assignment PIN_J12 -to adc0
	set_location_assignment PIN_L10 -to adc1
	set_location_assignment PIN_H6 -to osc
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[32]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_clk_ifc
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[33]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[31]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[30]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[29]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[28]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[27]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[26]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[25]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[24]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[23]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[22]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[21]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[20]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[19]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[18]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[17]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[16]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[15]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[14]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[13]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[12]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[11]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[10]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[9]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[8]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[7]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[6]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[5]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[4]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[3]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[2]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[1]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_data[0]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_in_ready
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_clk_ifc
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[20]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[19]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[18]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[17]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[16]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[15]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[14]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[13]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[12]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[11]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[10]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[9]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[8]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[7]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[6]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[5]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[4]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[3]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[2]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[1]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_data[0]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_ready
	set_instance_assignment -name IO_STANDARD "1.8 V" -to BD_out_valid
	set_instance_assignment -name IO_STANDARD "1.8 V" -to _BD_in_valid
	set_instance_assignment -name IO_STANDARD "1.8 V" -to adc0
	set_instance_assignment -name IO_STANDARD "1.8 V" -to adc1
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_in[10]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_in[9]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_in[8]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_in[7]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_in[6]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_in[5]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_in[4]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_in[3]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_in[2]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_in[1]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_in[0]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_in_clk
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_out[10]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_out[9]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_out[8]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_out[7]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_out[6]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_out[5]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_out[4]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_out[3]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_out[2]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_out[1]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_out[0]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_out_clk
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_ready_in
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_ready_out
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_valid_in
	set_instance_assignment -name IO_STANDARD "1.8 V" -to bot_valid_out
	set_instance_assignment -name IO_STANDARD "1.8 V" -to osc
	set_instance_assignment -name IO_STANDARD "1.8 V" -to pReset
	set_instance_assignment -name IO_STANDARD "1.8 V" -to sReset
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_in[10]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_in[9]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_in[8]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_in[7]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_in[6]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_in[5]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_in[4]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_in[3]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_in[2]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_in[1]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_in[0]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_in_clk
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_out[10]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_out[9]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_out[8]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_out[7]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_out[6]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_out[5]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_out[4]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_out[3]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_out[2]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_out[1]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_out[0]
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_out_clk
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_ready_in
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_ready_out
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_valid_in
	set_instance_assignment -name IO_STANDARD "1.8 V" -to top_valid_out
	set_instance_assignment -name PARTITION_HIERARCHY root_partition -to | -section_id Top

	# Including default assignments
	set_global_assignment -name TIMEQUEST_MULTICORNER_ANALYSIS ON -family "MAX 10"
	set_global_assignment -name TIMEQUEST_REPORT_WORST_CASE_TIMING_PATHS OFF -family "MAX 10"
	set_global_assignment -name TIMEQUEST_CCPP_TRADEOFF_TOLERANCE 0 -family "MAX 10"
	set_global_assignment -name TDC_CCPP_TRADEOFF_TOLERANCE 0 -family "MAX 10"
	set_global_assignment -name TIMEQUEST_DO_CCPP_REMOVAL ON -family "MAX 10"
	set_global_assignment -name TIMEQUEST_SPECTRA_Q OFF -family "MAX 10"
	set_global_assignment -name SYNTH_TIMING_DRIVEN_SYNTHESIS ON -family "MAX 10"
	set_global_assignment -name SYNCHRONIZATION_REGISTER_CHAIN_LENGTH 2 -family "MAX 10"
	set_global_assignment -name SYNTH_RESOURCE_AWARE_INFERENCE_FOR_BLOCK_RAM ON -family "MAX 10"
	set_global_assignment -name OPTIMIZE_HOLD_TIMING "ALL PATHS" -family "MAX 10"
	set_global_assignment -name OPTIMIZE_MULTI_CORNER_TIMING ON -family "MAX 10"
	set_global_assignment -name AUTO_DELAY_CHAINS ON -family "MAX 10"

	# Commit assignments
	export_assignments

	# Close project
	if {$need_to_close_project} {
		project_close
	}
}
