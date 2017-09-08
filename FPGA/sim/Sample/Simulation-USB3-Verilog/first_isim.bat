REM First Simulation Batch File
REM $Rev$ $Date$

REM Edit path for settings32/64, depending on architecture
call %XILINX%\..\settings64.bat

fuse -intstyle ise ^
     -incremental ^
     -lib unisims_ver ^
     -lib unimacro_ver ^
     -lib xilinxcorelib_ver ^
     -i ./oksim ^
     -o first_isim.exe ^
     -prj first_isim.prj ^
     work.FIRST_TEST work.glbl
first_isim.exe -gui -tclbatch first_isim.tcl -wdb first_isim.wdb