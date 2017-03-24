cmake_minimum_required(VERSION 3.0)
project(TEST CXX)
include_directories(../../../)
SET(CMAKE_CXX_FLAGS "-std=c++11 -lpthread")

add_executable(MutexBuffer_test MutexBuffer_test.cpp)

add_executable(Binary_test Binary_test.cpp ../util.cpp ../Binary.cpp)
