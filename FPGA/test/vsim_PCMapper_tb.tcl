#cd /home/aneckar/SharedWithCADMonkey/pystorm/FPGA/test
vlog -reportprogress 300 -work work /home/aneckar/SharedWithCADMonkey/pystorm/FPGA/test/PCMapper_tb.sv
vsim PCMapper_tb
add wave -position insertpoint sim:/PCMapper_tb/PC_in_ext/*
add wave -position insertpoint sim:/PCMapper_tb/parser/BD_data_out/*
add wave -position insertpoint sim:/PCMapper_tb/parser/conf_channel_out/*
add wave -position insertpoint  \
sim:/PCMapper_tb/parser/word_type
add wave -position insertpoint  \
sim:/PCMapper_tb/parser/conf_array_id
add wave -position insertpoint  \
sim:/PCMapper_tb/parser/leaf_code
add wave -position insertpoint  \
sim:/PCMapper_tb/parser/is_chan
add wave -position insertpoint  \
sim:/PCMapper_tb/parser/FPGA_or_BD
add wave -position insertpoint sim:/PCMapper_tb/dut/SF_conf/*
add wave -position insertpoint sim:/PCMapper_tb/dut/SG_program_mem/*
add wave -position insertpoint sim:/PCMapper_tb/dut/SG_conf/*
add wave -position insertpoint sim:/PCMapper_tb/dut/TM_conf/*
add wave -position insertpoint sim:/PCMapper_tb/dut/TS_conf/*
add wave -position insertpoint sim:/PCMapper_tb/dut/BD_conf/*
add wave -position insertpoint  \
