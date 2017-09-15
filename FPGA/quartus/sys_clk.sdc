############################################################################
# ZEM5305 sys_clk - Quartus Constraints File
#
# Timing constraints for the ZEM5305 sys_clk.
#
# Copyright (c) 2004-2015 Opal Kelly Incorporated
# $Rev: 584 $ $Date: 2010-10-01 11:14:42 -0500 (Fri, 01 Oct 2010) $
############################################################################

create_clock -name {sys_clk_p} -period 10.000 -waveform { 0.000 5.000 } [get_ports {sys_clk_p}]

#set_clock_groups -asynchronous -group [get_clocks {sys_clk_p}] -group [get_clocks {okUH0 virt_okUH0 ok_ifc|okHI|ok_altera_pll0|altera_pll_i|general[0].gpll~PLL_OUTPUT_COUNTER|divclk}]
