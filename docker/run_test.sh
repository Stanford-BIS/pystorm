#!/bin/bash
# These are the commands that a developer would type to build and run
# the automated tests for PyStorm

export LD_LIBRARY_PATH=/home/git/pystorm/bddriver/lib
cd /home/git/pystorm/bddriver/build/test
./bddriver_test
