#!/bin/bash
# These are the commands that a developer would type to build and run
# the automated tests for PyStorm

cd /home/git/pystorm/bddriver
./build_driver.sh
export LD_LIBRARY_PATH=/home/git/pystorm/bddriver/lib
cd build/test
./bddriver_test
