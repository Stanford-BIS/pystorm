cmake_minimum_required(VERSION 3.0)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)

project(bddriver CXX)
set(TARGET_NAME "Driver")

############################################################################
#
# Common setup and sanity checks
#
############################################################################

# This has to be done after 'project' is set
include(CheckRequirements)
include(Common)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
    "Choose the type of build, options are : Debug Release"
    FORCE)
endif()

if(NOT HOST_IS_VALID)
    message(FATAL_ERROR "Build does not support host '"
            ${CMAKE_HOST_SYSTEM_NAME} "'")
endif()

############################################################################
#
# Build setup
#
############################################################################

# Additional packages
find_package(Threads REQUIRED)

############################################################################
#
# Comm type selection
# Valid options are OPALKELLY, USB or SOFT
#
############################################################################
if(NOT BD_COMM_TYPE)
    set(BD_COMM_TYPE "SOFT" CACHE STRING
    "Choose the type of build, options are : \"OPALKELLY\", \"USB\", \"SOFT\" or \"MODEL\""
    FORCE)
endif()

# Opal-Kelly support
if(BD_COMM_TYPE MATCHES "OPALKELLY")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../FPGA/ext/opalkelly/include)
    find_library(OK_LIBRARY okFrontPanel HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../FPGA/ext/opalkelly/lib)
    add_definitions(-DBD_COMM_TYPE_OPALKELLY)
    message(STATUS "COMM is Opal Kelly: " ${OK_LIBRARY})
elseif(BD_COMM_TYPE MATCHES "USB")
    message(STATUS "COMM is USB")
    add_definitions(-DBD_COMM_TYPE_USB)
elseif(BD_COMM_TYPE MATCHES "SOFT")
    message(STATUS "COMM is SOFT")
    add_definitions(-DBD_COMM_TYPE_SOFT)
elseif(BD_COMM_TYPE MATCHES "MODEL")
    message(STATUS "COMM is MODEL")
    add_definitions(-DBD_COMM_TYPE_MODEL)
endif()

# Project settings
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
if(CXX_IS_MSVC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
endif()

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
set(HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Driver.h)
set(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Driver.cpp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/comm)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/common)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/decoder)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/encoder)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/model)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)
include_directories(${INCLUDE_DIRS})

# This should be before TARGET is set
SetupSharedLibrary()

# Define the library being built
add_library(${TARGET_NAME} SHARED
            ${HEADER_FILES}
            ${SRC_FILES}
           )
target_link_libraries(${TARGET_NAME} ${CMAKE_THREADS_LIB_INIT} ${OK_LIBRARY})
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD ${PYSTORM_CXX_STANDARD})
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)