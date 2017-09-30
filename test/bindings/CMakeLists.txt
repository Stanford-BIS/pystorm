cmake_minimum_required(VERSION 3.0)
set(PROJECT_NAME_STR pystorm_test)
set(PRJ_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

include(CheckRequirements)
include(Common)

set(PYSTORM_BASE_LIB_DIR ${PRJ_ROOT_DIR}/lib/$<CONFIG>)

#-------------------
# Set global variables
#-------------------
set(LIB_BASE_DIR ${PRJ_ROOT_DIR}/lib)

#-------------------
# Install and build external dependencies 
#-------------------

find_package(PythonInterp 3.6)

#-------------------
# Build Test
#-------------------
enable_testing()

add_test(NAME PyStorm_test
         COMMAND ${PYTHON_EXECUTABLE} -m unittest PyStorm_test
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python/3_5)

add_test(NAME PyDriver_test
         COMMAND ${PYTHON_EXECUTABLE} PyDriver_test.py
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python/3_5)

if(WIN32)
    set_property(TEST PyStorm_test PROPERTY
    ENVIRONMENT
        "PYTHONPATH=${PRJ_ROOT_DIR}/lib/Release;${PRJ_ROOT_DIR}/lib/Debug"
    )
    set_property(TEST PyStorm_test APPEND PROPERTY
    ENVIRONMENT
        "PATH=${PRJ_ROOT_DIR}/lib/Release;${PRJ_ROOT_DIR}/lib/Debug;%PATH%"
    )
    set_property(TEST PyDriver_test PROPERTY
    ENVIRONMENT
        "PYTHONPATH=${PRJ_ROOT_DIR}/lib/Release;${PRJ_ROOT_DIR}/lib/Debug"
    )
    set_property(TEST PyDriver_test APPEND PROPERTY
    ENVIRONMENT
        "PATH=${PRJ_ROOT_DIR}/lib/Release;${PRJ_ROOT_DIR}/lib/Debug;%PATH%"
    )
else()
    set_property(TEST PyStorm_test PROPERTY
    ENVIRONMENT
    "PYTHONPATH=${PYSTORM_BASE_LIB_DIR}"
    )
    set_property(TEST PyStorm_test APPEND PROPERTY
    ENVIRONMENT
    "LD_LIBRARY_PATH=${PYSTORM_BASE_LIB_DIR}")
    set_property(TEST PyDriver_test PROPERTY
    ENVIRONMENT
    "PYTHONPATH=${PYSTORM_BASE_LIB_DIR}"
    )
    set_property(TEST PyDriver_test APPEND PROPERTY
    ENVIRONMENT
    "LD_LIBRARY_PATH=${PYSTORM_BASE_LIB_DIR}"
    )
endif()
