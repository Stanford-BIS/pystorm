set_global_assignment -entity "BDIOPLL" -library "lib_BDIOPLL" -name IP_TOOL_NAME "altera_pll"
set_global_assignment -entity "BDIOPLL" -library "lib_BDIOPLL" -name IP_TOOL_VERSION "17.0"
set_global_assignment -entity "BDIOPLL" -library "lib_BDIOPLL" -name IP_TOOL_ENV "mwpim"
set_global_assignment -library "lib_BDIOPLL" -name SPD_FILE [file join $::quartus(sip_path) "BDIOPLL.spd"]

set_global_assignment -library "lib_BDIOPLL" -name MISC_FILE [file join $::quartus(sip_path) "BDIOPLL_sim/BDIOPLL.vo"]
