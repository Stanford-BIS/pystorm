cmake_minimum_required(VERSION 3.0)
project(PyStorm CXX)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# This has to be done after 'project' is set
include(CheckRequirements)
include(Common)

# Enable testing
enable_testing()

# Load packages
find_package(PythonInterp 3.5)
find_package(PythonLibs 3.5 REQUIRED)

############################################################################
#
# Entry function to build on Linux/Darwin/Windows
# We need to make sure that the compiler is correct
#
############################################################################

function(build_common)
    # Build and install external dependencies into lib/ directory
    link_directories(${PROJECT_SOURCE_DIR}/lib)

    # External modules
    if (CXX_IS_VALID)
        add_subdirectory(${PROJECT_SOURCE_DIR}/ext)
        add_subdirectory(${PROJECT_SOURCE_DIR}/src)
        add_subdirectory(${PROJECT_SOURCE_DIR}/test)
    endif()
endfunction()

############################################################################
#
# Entry function to build on Linux, Mac OSX and Windows 10
#
############################################################################

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
    "Choose the type of build, options are : Debug Release"
    FORCE)
endif()

if(NOT BD_COMM_TYPE)
    set(BD_COMM_TYPE "SOFT" CACHE STRING
    "Choose the type of build, options are : \"OPALKELLY\", \"USB\", \"MODEL\" or \"SOFT\""
    FORCE)
endif()

if(HOST_IS_VALID)
    build_common()
else()
    message(FATAL_ERROR "Build does not support host '"
        ${CMAKE_HOST_SYSTEM_NAME} "'")
endif()
