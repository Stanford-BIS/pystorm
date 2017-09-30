cmake_minimum_required(VERSION 3.0)
set(TARGET_NAME "Hal")

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

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
set(HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Hal.h)
set(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Hal.cpp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/common)

include_directories(${PRJ_ROOT_DIR}/include)
include_directories(${INCLUDE_DIRS})

# Define the library being built
add_library(${TARGET_NAME} SHARED
    ${HEADER_FILES}
    ${SRC_FILES}
    )
target_link_libraries(${TARGET_NAME} ${CMAKE_THREADS_LIB_INIT})
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD ${PYSTORM_CXX_STANDARD})
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
