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

Read quartus/OK_README.txt for some good info if setting up a project from scratch
using the OK board.

If you run into weird timing problems with the OK stuff, I recommend just repeating the
process in OK_README.txt, replacing Counters.v with OKCoreBD (or whatever your toplevel is)

- Load the .qpf in /quartus.
- You might need to add the license in /ext/opalkelly/FrontPanelHDL.
- Everything else should already be set up! 
- Press the big blue button to compile the project. 
  Hopefully, there are no problems.
- After that, generate the .rbf bitfile.

That's it! We don't program the board through Quartus

===========================================
ModelSim (ASE: Altera version) instructions:

Don't bother with whatever the Quartus->ModelSim integration is supposed to do.
Run Modelsim by itself.

- Open the ModelSim GUI
- cd to /test
- create a new library using the GUI, call it work (the default)
- in the terminal: "source compile_all.tcl"
- To run something with altera IP cores ('megafunctions'), you need to invoke vsim like:
  vsim -L altera_mf_ver -L altera_mf path/to/your/compiled/module

That's it!
