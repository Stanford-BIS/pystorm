onerror {resume}

divider add "FrontPanel Control"
wave add /FIRST_TEST/okUH(0)

divider add "Simulation"
wave add -radix hex r1
wave add -radix hex r2
wave add -radix hex sum

divider add "First"
wave add -radix hex /FIRST_TEST/dut/ep01wire
wave add -radix hex /FIRST_TEST/dut/ep02wire
wave add -radix hex /FIRST_TEST/dut/ep21wire

run 8us;
