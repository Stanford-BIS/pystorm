vlog -work work OKIfc_tb.sv
vsim -L altera_ver -L lpm_ver -L sgate_ver -L altera_mf_ver -L altera_lnsim_ver -L cyclonev_ver -L cyclonev_hssi_ver -L cyclonev_pcie_hip_ver work.OKIfc_tb
add wave -position insertpoint sim:/OKIfc_tb/dut/*
add wave -position insertpoint sim:/OKIfc_tb/downstream_sink/in/*
add wave -position insertpoint sim:/OKIfc_tb/upstream_source/out/*
