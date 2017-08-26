cmake_minimum_required(VERSION 3.0)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)

project(bdhal CXX)
set(TARGET_NAME "Hal")

############################################################################
#
# Common setup and sanity checks
#
############################################################################

# This has to be done after 'project' is set
include(CheckRequirements)
include(Common)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
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

# Project settings
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
if(CXX_IS_MSVC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
endif()

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
set(HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Hal.h)
set(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Hal.cpp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/common)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)
include_directories(${INCLUDE_DIRS})

# This should be before TARGET is set
SetupSharedLibrary()

# Define the library being built
add_library(${TARGET_NAME} SHARED
    ${HEADER_FILES}
    ${SRC_FILES}
    )
target_link_libraries(${TARGET_NAME} ${CMAKE_THREADS_LIB_INIT})
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 11)