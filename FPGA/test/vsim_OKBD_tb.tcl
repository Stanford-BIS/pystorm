source ../quartus/BDIOPLL_sim/mentor/msim_setup.tcl
vlog -work work OKBD_tb.sv
vsim -L altera_ver -L lpm_ver -L sgate_ver -L altera_mf_ver -L altera_lnsim_ver -L cyclonev_ver -L cyclonev_hssi_ver -L cyclonev_pcie_hip_ver work.OKBD_tb
