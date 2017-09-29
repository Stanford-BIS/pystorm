#!/bin/bash

# Script assumes that `quartus` is in path

# Setup OK license
export LM_LICENSE_FILE=$PWD/okAlteraLicense.dat:$LM_LICENSE_FILE

# Generate the project
quartus_sh -t OKCoreBD.tcl

# Compile the project
quartus_sh --flow compile OKCoreBD

# Generate timing report
quartus_sta --64bit --report_script=timing_report.tcl OKCoreBD

# Check the report for negative slack