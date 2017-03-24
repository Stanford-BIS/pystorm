cmake_minimum_required(VERSION 3.0)
project(TEST CXX)
include_directories(../../)
SET(CMAKE_CXX_FLAGS "-std=c++11 -lpthread -m64 -Wall -O3")

add_executable(Encoder_test Encoder_test.cpp ../Encoder.cpp ../../common/BDPars.cpp ../../common/HWLoc.cpp ../../common/Binary.cpp ../../common/util.cpp)
