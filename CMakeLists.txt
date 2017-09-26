cmake_minimum_required(VERSION 3.0)
project(PyStorm CXX)


# tell cmake it can find modules in ./cmake/ directory
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(CheckRequirements)
include(Common)

# Enable testing
enable_testing()

if(NOT BD_COMM_TYPE)
    set(BD_COMM_TYPE "OPALKELLY" CACHE STRING
    "Choose the type of build, options are : \"OPALKELLY\", \"USB\", \"MODEL\" or \"SOFT\""
    FORCE)
endif()

# Build and install external dependencies into lib/ directory
link_directories(${PROJECT_SOURCE_DIR}/lib)
# External modules
add_subdirectory(${PROJECT_SOURCE_DIR}/ext)
# Project directories
add_subdirectory(${PROJECT_SOURCE_DIR}/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/test)

set(CMAKE_CXX_STANDARD ${PYSTORM_CXX_STANDARD} CACHE STRING "" FORCE)
message("Generated with config types: ${CMAKE_CONFIGURATION_TYPES}")
message("Selected configuration: ${CMAKE_BUILD_TYPE}")
