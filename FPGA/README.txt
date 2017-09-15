===========================================
Explanation of directores

/src has the HDL for the FPGA design. Includes anything that we wrote.

/ext has external HDL and source code
/ext/opalkelly/FrontPanelHDL has the (encrypted) FrontPanel IP cores
/ext/opalkelly/Simulation has HDL behavioral models for the okHost and endpoints 
/ext/lib and /ext/include are for the OK software libraries

/quartus contains Quartus project files (including constraints)
as well as HDL for the altera IP cores (e.g. memories)

/test has HDL test harnesses. But most are with their source for now

===========================================
Quartus instructions:

Our source code uses includes for dependencies between files.
The quartus project includes all of the IP core files, both 
for Altera and Opal Kelly (OK) IP.

- Load the .qpf in /quartus.
- You might need to add the OK license
    tools->License Setup->[add /quartus/okAlteraLicense.dat]
- Everything else should already be set up! 
- Press the big blue triangle to compile the project. 
  Hopefully, there are no problems.
- After that, generate the bitfile
  it goes to output_files/<project name>.rbf

That's it! We don't program the OK board through Quartus

If you get in trouble:

Read quartus/OK_README.txt for some good info for setting up a project from scratch
using the OK board.

NOTE THAT A LOT OF SAMPLE STUFF WILL BREAK IF YOU MOVE THE OK MODULES FROM
THE TOP OF THE PROJECT HIERARCHY!!!!! (or from where they are in my project)

In my project, OK stuff is in OKIFc:ok_ifc|OKHost:okHI.
If you move it, you have to fix okHost.sdc and the PLL stuff in <project>.qsf

Quartus is really touchy. Don't edit the .qsfs of open projects.

===========================================
ModelSim (ASE: Altera version) instructions:

There's a "`ifdef SIMULATION" switch in /src files to include external IP for ModelSim.
Your testbench should include the /src file it needs, 
and have "`define SIMULATION" as the first line in the file

Don't bother with whatever the Quartus->ModelSim integration is supposed to do.
Run Modelsim by itself.

- Open the ModelSim GUI
- cd to /test
- in the terminal: "source vsim_OKCoreBD_tb.tcl"
  you can crib something similar for other projects
  I'm not sure how necessary the msim_setup.ini for the PLL is
- To run something with most altera IP cores ('megafunctions'), you need to invoke vsim like:
  vsim -L altera_mf_ver path/to/your/compiled/module
- To run something with PLLs or IO stuff (God help you):
  vsim -L altera_ver -L lpm_ver -L sgate_ver -L altera_mf_ver -L altera_lnsim_ver \
  -L cyclonev_ver -L cyclonev_hssi_ver -L cyclonev_pcie_hip_ver path/to/your/compiled/module
