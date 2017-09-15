############################################################################
# ZEM5305 - Quartus Constraints File
#
# Timing constraints for the ZEM5305.
#
# Copyright (c) 2004-2015 Opal Kelly Incorporated
# $Rev:$ $Date:$
############################################################################


#**************************************************************
# Time Information
#**************************************************************

set_time_format -unit ns -decimal_places 3



#**************************************************************
# Create Clock
#**************************************************************

create_clock -name {okUH0} -period 9.920 -waveform { 0.000 4.960 } [get_ports {okUH[0]}]
create_clock -name {virt_okUH0} -period 9.920 -waveform { 0.000 4.960 } 


#**************************************************************
# Create Generated Clock
#**************************************************************

derive_pll_clocks


#**************************************************************
# Set Clock Uncertainty
#**************************************************************

derive_clock_uncertainty


#**************************************************************
# Set Input Delay
#**************************************************************

set_input_delay -add_delay -max -clock [get_clocks {virt_okUH0}]  8.000  [get_ports {okUH[*]}]
set_input_delay -add_delay -min -clock [get_clocks {virt_okUH0}]  10.000 [get_ports {okUH[*]}]
set_input_delay -add_delay -max -clock [get_clocks {virt_okUH0}]  8.000  [get_ports {okUHU[*]}]
set_input_delay -add_delay -min -clock [get_clocks {virt_okUH0}]  2.000  [get_ports {okUHU[*]}]

#**************************************************************
# Set Output Delay
#**************************************************************

set_output_delay -add_delay -max -clock [get_clocks {okUH0}]  2.000  [get_ports {okHU[*]}]
set_output_delay -add_delay -min -clock [get_clocks {okUH0}]  -0.500 [get_ports {okHU[*]}]
set_output_delay -add_delay -max -clock [get_clocks {okUH0}]  2.000  [get_ports {okUHU[*]}]
set_output_delay -add_delay -min -clock [get_clocks {okUH0}]  -0.500  [get_ports {okUHU[*]}]

#**************************************************************
# Set Multicycle Path
#**************************************************************

set_multicycle_path -setup -from [get_clocks {ok_ifc|okHI|ok_altera_pll0|altera_pll_i|general[0].gpll~PLL_OUTPUT_COUNTER|divclk}] -to [get_clocks {okUH0}] 2

# (C) 2001-2014 Altera Corporation. All rights reserved.
# Your use of Altera Corporation's design tools, logic functions and other 
# software and tools, and its AMPP partner logic functions, and any output 
# files any of the foregoing (including device programming or simulation 
# files), and any associated documentation or information are expressly subject 
# to the terms and conditions of the Altera Program License Subscription 
# Agreement, Altera MegaCore Function License Agreement, or other applicable 
# license agreement, including, without limitation, that your use is for the 
# sole purpose of programming logic devices manufactured by Altera and sold by 
# Altera or its authorized distributors.  Please refer to the applicable 
# agreement for further details.


# +---------------------------------------------------
# | Cut the async clear paths
# +---------------------------------------------------
set aclr_counter 0
set clrn_counter 0
set aclr_collection [get_pins -compatibility_mode -nocase -nowarn *|alt_rst_sync_uq1|altera_reset_synchronizer_int_chain*|aclr]
set clrn_collection [get_pins -compatibility_mode -nocase -nowarn *|alt_rst_sync_uq1|altera_reset_synchronizer_int_chain*|clrn]
foreach_in_collection aclr_pin $aclr_collection {
    set aclr_counter [expr $aclr_counter + 1]
}
foreach_in_collection clrn_pin $clrn_collection {
    set clrn_counter [expr $clrn_counter + 1]
}
if {$aclr_counter > 0} {
    set_false_path -to [get_pins -compatibility_mode -nocase *|alt_rst_sync_uq1|altera_reset_synchronizer_int_chain*|aclr]
}

if {$clrn_counter > 0} {
    set_false_path -to [get_pins -compatibility_mode -nocase *|alt_rst_sync_uq1|altera_reset_synchronizer_int_chain*|clrn]
}
