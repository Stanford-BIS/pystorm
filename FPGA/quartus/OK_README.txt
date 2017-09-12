Opal Kelly Samples README
=========================
  1. General
  2. Simulation
  3. Pre-built sample bitfiles
  4. Building sample bitfiles using Xilinx ISE
  5. Building sample bitfiles using Xilinx Vivado
  6. Building sample bitfiles using Altera Quartus



1. GENERAL
==========
Samples here are provided as a starting point.  They aren't intended as
full-blown applications with error-checking and so on.

To build the C/C++ samples, Visual Studio projects (Windows) and Makefiles
(Linux and Mac OS X) have been provided.  These generally reference the 
native compilers.


   okFP_SDK
   --------
   The environment variable "okFP_SDK" is set by the FrontPanel installer 
   for Windows. On Mac and Linux, this variable has a default override within
   the Makefiles so you do not need to set it if you build the samples in place.
   
   By default, this is set to:
      C:\Program Files\Opal Kelly\FrontPanelUSB\API
   
   The Visual Studio projects reference this for the following files:
      $okFP_SDK/include/okFrontPanelDLL.h
      $okFP_SDK/lib/Win32/okFrontPanel.lib    (for 32-bit architecture)
      $okFP_SDK/lib/x64/okFrontPanel.lib      (for 64-bit architecture)
   
   Furthermore, when you run the application, you will need to copy the DLL 
   to the application's working directory. The DLL may be found here:
      $okFP_SDK/lib/Win32/okFrontPanel.dll    (for 32-bit architecture)
      $okFP_SDK/lib/x64/okFrontPanel.dll      (for 64-bit architecture)
   
   
   Windows Builds
   --------------
   The Samples folder installed in the installation directory may be copied
   to another location on your filesystem. The Visual Studio Solution file may 
   then be opened directly and the samples should all build from there.


   FrontPanel SDK Library (DLL / Shared-Object)
   --------------------------------------------
   Samples that have an executable will also require that you copy the 
   DLL (or .so or .a for Linux/Mac, respectively) to the executable directory.
   This can be found in the API directory where you installed FrontPanel.
   
   For Linux, the .so file will need to be in your LD_LIBRARY_PATH which does 
   not necessarily include the present directory ("."). Therefore, you can set
   it prior to running the executable. For example:
   
      $ LD_LIBRARY_PATH=. ./PipeTest
   
   This will set the LD_LIBRARY_PATH to include the current directory for the
   one-time execution of the PipeTest executable.
   
   
   wxWidgets
   ---------
   For Windows applications that require wxWidgets, you will need to download
   and build wxWidgets (http://www.wxwidgets.org).  You will also need to set
   a system environment variable to wherever you installed the base tree
   for wxWidgets, e.g.:
      WXWIN = c:\wxWidgets-3.0.0

   The Visual Studio projects also reference different locations for the
   libraries depending on architecture:
      32-bit builds: $(WXWIN)\lib\vc120_lib
      64-bit builds: $(WXWIN)\lib\vc120_x64_lib

   There are a number of ways to build wxWidgets.  Please refer to the wxWidgets
   documentation.  But here's how we do it on Windows (e.g., wxWidgets 3.0.0).
   This puts the resulting libraries in the directories listed above.
      1. Download wxWidgets from http://www.wxwidgets.org
      2. Uncompress to c:\wxWidgets-3.0.0
      3. cd c:\wxWidgets-3.0.0\build\msw
      4. For 32-bit build (must use a 32-bit Visual Studio Command Line)
         nmake -f makefile.vc COMPILER_PREFIX=vc120 BUILD=release -sa
      5. For 64-bit build (must use a 64-bit Visual Studio Command Line)
         nmake -f makefile.vc COMPILER_PREFIX=vc120 BUILD=release TARGET_CPU=X64 -sa



The table below lists each of the samples included with FrontPanel. 
Depending on the specific capabilities highlighted, they include different
sources.  The included sources are listed below for each sample.

Sample          Collateral Included
---------------------------------------------------------------------------
First           XFP   Simulation   Verilog   VHDL
Counters        XFP                Verilog   VHDL
Controls        XFP                Verilog
PipeTest                           Verilog   VHDL   C++
DES                   Simulation   Verilog   VHDL   C++/Python/Java/Ruby
RAMTester                          Verilog          C++
FlashLoader                                         C++
DeviceChange                                        C++/wxWidgets

   + XFP - FrontPanel Application XML (XFP) provided
   + Simulation - Behavioral simulation is provided
   + Verilog - FPGA Verilog description is provided
   + VHDL - FPGA VHDL description is provided
   + C++ - C++ application using the FrontPanel DLL is included
     (Python, Java, Ruby, etc. may also be provided)
   + C++/wxWidgets - C++ application that uses wxWidgets GUI library




2. SIMULATION
=============
The following samples have associated simulation versions:

   + First (Verilog and VHDL)
   + DES   (Verilog)

Simulation versions are setup very similar to an actual FPGA-targeted 
project, but might differ in a specific detail or two.  In particular:

   + Simulation projects have the additional "*_tf.v" file which 
     contains the test fixture and attaches the FPGA pinout to the 
     host simulation.  The host simulation represents everything from the
     FPGA host interface pins back to the PC application code.
     
   + Simulation projects have the additional "okHostCalls.v" file which 
     contains Verilog or HDL models for the functions in the FrontPanel API.
   
   + Simulation projects include a Modelsim "do file" for executing the 
     simulation compiler and performing the test bench setup.




3. PRE-BUILT SAMPLE BITFILES
============================
Opal Kelly provides pre-built sample bitfiles for quick-start at the
following URL:

   https://pins.opalkelly.com/downloads

Each bundle contains bitfiles specific to a particular Opal Kelly 
module and is based on the most recent minor release and is dated.




4. BUILDING SAMPLES BITFILES USING XILINX ISE
=============================================
This will briefly guide you through the setup of a Xilinx
Project Navigator project and help you build this sample on your own.
The same process can be extended to any of the included samples as well
as your own projects.

This file is written for the Counters project, but with appropriate changes,
it will extend easily to the others.

If you are new to ISE Project Navigator, you should review the Xilinx 
documentation on that software.  At least some familiarity with ISE is
assumed here.


Step 1: Create a new Project
----------------------------
Within Project Navigator, create a new project.  This will also create
a new directory to contain the project.  You will be copying files into
this new location.

Be sure to select the correct FPGA device for your particular board:
   XEM3001v2         => xc3s400-4pq208
   XEM3010-1500      => xc3s1500-4fg320
   XEM3010-1000      => xc3s1000-4fg320
   XEM3005-1200      => xc3s1200e-4ft256
   XEM3050           => xc3s4000-5fg676
   XEM5010           => xc5vlx50-1ff676
   XEM6001           => XC6SLX16-2FTG256
   XEM6002           => XC6SLX9-2FTG256
   XEM6006           => XC6SLX16-2FTG256
   XEM6010-LX45      => xc6slx45-2fgg484
   XEM6010-LX150     => xc6slx150-2fgg484
   XEM6110-LX45      => xc6slx45-2fgg484
   XEM6110-LX150     => xc6slx150-2fgg484
   XEM6310-LX45      => xc6slx45-2fgg484
   XEM6310-LX150     => xc6slx150-2fgg484
   XEM6310MT-LX45T   => xc6slx45t-2fgg484
   XEM6310MT-LX150T  => xc6slx150t-2fgg484
   XEM6320-LX130T    => xc6vlx130t-1-ff1156
   XEM7350-K70T      => xc7k70t-1fbg676
   XEM7350-K160T     => xc7k160t-1ffg676
   XEM7350-K410T     => xc7k410t-1ffg676


Step 2: Copy source files to Project
------------------------------------
Copy the files from the sample directory to your new Project directory.
For the Counters (XEM3010-Verilog) sample, these are:
   + Counters.v     - Counters Verilog source.
   + xem3010.ucf    - Constraints file for the XEM3010 board.


Step 3: Copy Opal Kelly FrontPanel HDL files
--------------------------------------------
From the FrontPanel installation directory, copy the HDL module files.
Be sure to use the files from the correct board model as some of the 
items are board-specific.  The README.txt file indicates which version
of Xilinx ISE was used to produce the NGC files.  Note that these files
are forward-compatible with newer ISE versions.
   + okLibrary.v    - Verilog library file
   + *.ngc          - Pre-compiled FrontPanel HDL modules
   
   
Step 4: Add sources to your Project
-----------------------------------
Within Project Navigator, select "Add Sources..." from the "Project" menu.
Add the following files to your project: (Note that you have already copied
these files to your project directory in the previous steps.  They are now
being added to your Project.)  You do NOT need to add the NGC files to your
project.
   + Counters.v
   + okLibrary.v
   + xem3010.ucf
   
   
Step 5: Generate Programming File
---------------------------------
Within Project Navigator, select your toplevel source (Counters.v in this
case) from the "Sources" list.  Then double-click on
"Generate Programming File" to have ISE build a programming file that you
can then download to your board using FrontPanel.

NOTE: For the XEM6110, it is critical that you configure BITGEN to 
      float unused pins.  If this is not done, the FPGA/PC communication
      will not work correctly.



4. BUILDING SAMPLES BITFILES USING XILINX VIVADO
=============================================
This will briefly guide you through the setup of a Xilinx
Vivado project and help you build this sample on your own.
The same process can be extended to any of the included samples as well
as your own projects.

This file is written for the Counters project, but with appropriate changes,
it will extend easily to the others.

If you are new to Vivado, you should review the Xilinx 
documentation on that software.  At least some familiarity with Vivado is
assumed here.


Step 1: Create a new Project
----------------------------
Within Project Navigator, create a new project.  This will also create
a new directory to contain the project.  You will be copying files into
this new location.

When prompted to select the project type, select "RTL project". Check 
the box "Do not specify sources at this time".

Be sure to select the correct FPGA device for your particular board:
   XEM7350-K70T      => xc7k70tfbg676-1
   XEM7350-K160T     => xc7k160tffg676-1
   XEM7350-K410T     => xc7k410tffg676-1
   XEM7001-A15T      => xc7a15tftg256-1


Step 2: Copy source files to Project
------------------------------------
Copy the Verilog file from the sample directory to your new Project directory.
For the Counters (XEM7350-Verilog) sample, this is:
   + Counters.v     - Counters Verilog source.
Copy the XDC constraints file from the Samples directory. For the XEM7350,
this is:
   + xem7350.xdc    - Constraints file for the XEM7350 board.


Step 3: Copy Opal Kelly FrontPanel HDL files
--------------------------------------------
From the FrontPanel installation directory, copy the HDL module files.
Be sure to use the files from the correct board model as some of the 
items are board-specific.  The README.txt file indicates which version
of Xilinx ISE was used to produce the NGC files.  Note that these files
are forward-compatible with newer ISE versions.
   + okLibrary.v    - Verilog library file
   + *.ngc          - Pre-compiled FrontPanel HDL modules (XEM7350)
   + *.v            - Encrypted FrontPanel HDL modules (XEM7001)
   
   
Step 4: Add sources to your Project
-----------------------------------
Within Project Navigator, select "Add Sources.."->"Add or Create Constraints" 
from the "File" menu. Add the XDC file to your project.
Select "Add Sources"->"Add or Create Design Sources".
Add the following files to your project: (Note that you have already copied
these files to your project directory in the previous steps.  They are now
being added to your Project.)
   + Counters.v
   + okLibrary.v

   XEM7001:
   + okBTPipeIn.v
   + okBTPipeOut.v
   + okCoreHarness.v
   + okPipeIn.v
   + okPIpeOut.v
   + okTriggerIn.v
   + okTriggerOut.v
   + okWireIn.v
   + okWireOut.v

   XEM7350:
   + *.ngc
Select "TFIFO64x8a_64x8b.ngc" in the Sources panel.
Select "Tools"->"Property Editor" and check the "IS_GLOBAL_INCLUDE" box.
   
   
Step 5: Generate Programming File
---------------------------------
Within Flow Navigator, click on "Generate Bitfile" to have Vivado build a
programming file that you can then download to your board using FrontPanel.
The default location of this file for Counters is:
counters.runs/impl_1/Counters.bit



6. BUILDING SAMPLES BITFILES USING ALTERA QUARTUS
=================================================
This will briefly guide you through the setup of an Altera Quartus
project and help you build this sample on your own. The same process
can be extended to any of the included samples as well as your own
projects.

This file is written for the Counters project, but with appropriate
changes, it will extend easily to the others.

If you are new to Quartus, you should review the Altera documentation
on that software.  At least some familiarity with Quartus is assumed
here.

The steps here depend on Quartus 13.1. Similar steps may be followed
for later versions of the software.


Step 0: Install the license file in the Quartus tools
-----------------------------------------------------
The Opal Kelly license file is required to utilize the encrypted HDL
provided to build an FPGA configuration image. You will only need to
follow these steps once for an installation of Quartus.

  A. Start Quartus and navigate to "Tools > License Setup..."

  B. Add the path to okAlteraLicense.dat in the License File field.
  
  C. Click OK
  
  D. Upon reopening the License Setup dialog, you should see a line for
     Vendor:29AE and Product:0120 listed in the "Licensed AMPP /
     MegaCore functions" window. This indicates a successful installation
     of the license.


Step 1: Create a new Project
----------------------------
Within Quartus, create a new project.  This will also create a new
directory to contain the project.  You will be copying files into
this new location.

Be sure to select the correct FPGA device and settings for your particular board:

  A. Within Quartus select the correct FPGA device for your board:
     ZEM4310   ->   EP4CE55F23C8N
     ZEM5305   ->   5CEFA2U19C8N
   
  B. Press "Device and Pin Options..." button.

  C. In Category > Configuration
     - Configuration scheme: 
       ZEM4310  ->  Fast Passive Parallel
       ZEM5305  ->  Passive Parallel x16
     
  D. In Category > Programing Files 
     - Select Raw Binary File (.rbf) to generate correct
       format for ZEM device configuration.
       
  E: In Category > Dual-Purpose Pins 
     - DCLK: Use as programming pin
     - All others: Use as regular I/O
     
  F. Within Quartus select "Assignments > Settings"
     - In Category > EDA Tool Settings, make sure Tool Names are all
       set to "<None>" for compatibility with the license.

Step 2: Copy source files to Project
------------------------------------
Copy the files from the sample directory to your new Project directory.
For the Counters (ZEM4310-Verilog) sample, these are:
   + Counters.v     - Counters Verilog source.
   + zem4310.qsf    - Quartus settings file for the ZEM4310 board.


Step 3: Add device settings to Project's settings file
------------------------------------------------------
Select "Assignments > Import Assignments"

In the Import Assignments window, use the browse button to navigate to and
select zem4310.qsf

Press the "Advanced..." button.

Under "Import Options" make sure the "Imported entity assignments replace
current entity assignments" option is checked.

If the design has already been compiled/synthesized Quartus may reject
some assignments. To fix this, select "Project > Clean Project..." and
click "OK". With the project cleaned repeat Step 3.

Step 4: Copy Opal Kelly FrontPanel HDL files
--------------------------------------------
From the FrontPanel installation directory, copy the HDL module files.
Be sure to use the files from the correct board model as some of the 
items are board-specific.  The README.txt file indicates which version
of Quartus was used to produce the files.  Note that these files
are forward-compatible with newer Quartus versions.

   + okHost.v             - Encrypted Verilog containing the okHost module.
   + okHost.sv            - Encrypted System Verilog containing the okHost
                            support modules.
   + okHost.sdc           - Timing constraints for the okHost I/O.
   + okEndpoints.v        - Encrypted Verilog containing the FrontPanel
                            endpoint modules.
   + okLibrary.v          - Verilog input output resistors and core harness(Verilog projects only).
   + okLibrary.vhd        - VHDL input output resisters and core harness (VHDL projects only).
   + okHostMicrocode.hex  - Encrypted memory for okHost.

Note: okHostMicrocode.hex must be placed in the same directory as the Quartus project file
to be found and used correctly.
   
   
Step 5: Add sources to your Project
-----------------------------------
Within Quartus, select "Add/Remove Files in Project..." from the "Project" menu.
Add the following files to your project: (Note that you have already copied
these files to your project directory in the previous steps.  They are now
being added to your Project.)  
   + Counters.v ( Set as top level entity. "Assignments > Settings... > General"
   + okHost.v
   + okHost.sv
   + okEndpoints.v
   + okLibrary.v   (Verilog projects only)
   + okLibrary.vhd (VHDL projects only)
   + okHost.sdc

Ensure that all sdc (synopsys design constraints) files are placed at the bottom
of the file list so that they are compiled last. This will reduce problems during
timing analysis.

Step 6: Compile Design
----------------------
Within Quartus, double-click on "Compile Design" to have Quartus build a *.rbf programming 
file that you can then download to your board using FrontPanel.
