add_wave_divider "FrontPanel Control"
add_wave /FIRST_TEST/okUH(0)

add_wave_divider "Simulation"
add_wave -radix hex r1
add_wave -radix hex r2
add_wave -radix hex sum

add_wave_divider "First"
add_wave -radix hex /FIRST_TEST/dut/ep01wire
add_wave -radix hex /FIRST_TEST/dut/ep02wire
add_wave -radix hex /FIRST_TEST/dut/ep21wire

run 8 us;
