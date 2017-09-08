REM First Simulation Batch File (Vivado)

call xelab work.FIRST_TEST work.glbl -prj first_isim.prj -L unisims_ver -L secureip -s sim_vivado -debug typical
xsim -g -t first_vivado.tcl -wdb first_vivado.wdb sim_vivado
