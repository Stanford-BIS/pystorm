# This is a Modelsim simulation script.
# To use:
#  + Start Modelsim
#  + At the command-line, CD to the directory where this file is.
#  + Type: "do thisfilename.do"
# $Rev$ $Date$

set PATH ./oksim

# Source files and testfixture
vlib work
vlog +incdir+$PATH First_tf.v

vlog First.v

vlog +incdir+$PATH $PATH/glbl.v
vlog +incdir+$PATH $PATH/okHost.v
vlog +incdir+$PATH $PATH/okWireIn.v
vlog +incdir+$PATH $PATH/okWireOut.v
vlog +incdir+$PATH $PATH/okWireOR.v

vsim -L unisims_ver -t ps FIRST_TEST -novopt +acc

#Setup waveforms
onerror {resume}
quietly WaveActivateNextPane {} 0

add wave -noupdate -divider {FrontPanel Control}
add wave -noupdate -format Logic {/FIRST_TEST/okUH[0]}

add wave -noupdate -divider Simulation
add wave -noupdate -format Literal -radix hexadecimal r1
add wave -noupdate -format Literal -radix hexadecimal r2
add wave -noupdate -format Literal -radix hexadecimal exp
add wave -noupdate -format Literal -radix hexadecimal sum

add wave -noupdate -divider First
add wave -noupdate -format Literal -radix hexadecimal /FIRST_TEST/dut/ep01wire
add wave -noupdate -format Literal -radix hexadecimal /FIRST_TEST/dut/ep02wire
add wave -noupdate -format Literal -radix hexadecimal /FIRST_TEST/dut/ep21wire

TreeUpdate [SetDefaultTree]
configure wave -namecolwidth 261
configure wave -valuecolwidth 58
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

update
WaveRestoreZoom {0 ns} {8 us}

run -all
