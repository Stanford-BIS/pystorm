cmake_minimum_required(VERSION 3.0)
set(TARGET_NAME "Driver")

############################################################################
#
# Common setup and sanity checks
#
############################################################################
set(PRJ_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(BD_LIBRARY_OUTPUT_PATH ${PRJ_ROOT_DIR}/lib)
list(APPEND CMAKE_MODULE_PATH ${PRJ_ROOT_DIR}/cmake)

# This has to be done after 'project' is set
include(CheckRequirements)
include(Common)

# This should be before TARGET is set
SetupCompilerOptions()
SetupBuildPaths()

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
    set(BD_COMM_TYPE "OPALKELLY" CACHE STRING
    "Choose the type of build, options are : \"OPALKELLY\", \"USB\", \"SOFT\" or \"MODEL\""
    FORCE)
endif()

# Opal-Kelly support
if(BD_COMM_TYPE STREQUAL "OPALKELLY")
    include_directories(${PRJ_ROOT_DIR}/FPGA/ext/opalkelly/include)
    find_library(OK_LIBRARY okFrontPanel HINTS ${PRJ_ROOT_DIR}/FPGA/ext/opalkelly/lib)
    add_definitions(-DBD_COMM_TYPE_OPALKELLY)
    message(STATUS "COMM is Opal Kelly: " ${OK_LIBRARY})
elseif(BD_COMM_TYPE STREQUAL "USB")
    message(STATUS "COMM is USB")
    add_definitions(-DBD_COMM_TYPE_USB)
elseif(BD_COMM_TYPE STREQUAL "SOFT")
    message(STATUS "COMM is SOFT")
    add_definitions(-DBD_COMM_TYPE_SOFT)
elseif(BD_COMM_TYPE STREQUAL "MODEL")
    message(STATUS "COMM is MODEL")
    add_definitions(-DBD_COMM_TYPE_MODEL)
endif()

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
set(HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Driver.h)
set(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Driver.cpp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/comm)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/common)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/decoder)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/encoder)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/model)

include_directories(${PRJ_ROOT_DIR}/include)
include_directories(${INCLUDE_DIRS})

# Define the library being built
add_library(${TARGET_NAME} SHARED
            ${HEADER_FILES}
            ${SRC_FILES}
           )
target_link_libraries(${TARGET_NAME} ${CMAKE_THREADS_LIB_INIT} ${OK_LIBRARY})
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD ${PYSTORM_CXX_STANDARD})
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
