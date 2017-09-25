cmake_minimum_required(VERSION 3.0)
set(PROJECT_NAME_STR pystorm_test)
project(${PROJECT_NAME_STR})

include(CheckRequirements)
include(Common)

set(PYSTORM_BASE_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)

#-------------------
# Set global variables
#-------------------
set(LIB_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)

#-------------------
# Install and build external dependencies 
#-------------------

find_package(PythonInterp 3.6)

#-------------------
# Build Test
#-------------------
enable_testing()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PYSTORM_BASE_LIB_DIR})
endif()

add_test(NAME Pystorm_test
         COMMAND ${PYTHON_EXECUTABLE} -m unittest PyStorm_test
         WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})

set_property(TEST Pystorm_test PROPERTY ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR}/python/3_5/:${PYSTORM_BASE_LIB_DIR};LD_LIBRARY_PATH=${PYSTORM_BASE_LIB_DIR}")
