#!/bin/bash
g++ -std=c++14 -g -O0 -L. -I../../src/bddriver -lDriver -Wl,-rpath=.  -o Driver_test Driver_test.cpp
