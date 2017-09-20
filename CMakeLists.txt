cmake_minimum_required(VERSION 3.0)
project(PyStorm CXX)


# tell cmake it can find modules in ./cmake/ directory
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
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
    add_subdirectory(${PROJECT_SOURCE_DIR}/ext)
    add_subdirectory(${PROJECT_SOURCE_DIR}/src)
    add_subdirectory(${PROJECT_SOURCE_DIR}/test)
endfunction()

############################################################################
#
# Entry function to build on Linux, Mac OSX and Windows 10
#
############################################################################
if(NOT BD_COMM_TYPE)
    set(BD_COMM_TYPE "MODEL" CACHE STRING
    "Choose the type of build, options are : \"OPALKELLY\", \"USB\", \"MODEL\" or \"SOFT\""
    FORCE)
endif()

build_common()

set(CMAKE_CXX_STANDARD ${PYSTORM_CXX_STANDARD} CACHE STRING "" FORCE)
message("Generated with config types: ${CMAKE_CONFIGURATION_TYPES}")
message("Selected configuration: ${CMAKE_BUILD_TYPE}")
