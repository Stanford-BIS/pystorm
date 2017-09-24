cmake_minimum_required(VERSION 3.0)

############################################################################
#
# Global variables
#
############################################################################

set(PROJECT_NAME_STR bddriver_app)
project(${PROJECT_NAME_STR} CXX)

include(CheckRequirements)
include(Common)

############################################################################
#
# Set modules required by all platforms
#
############################################################################

find_package(Threads REQUIRED)

############################################################################
#
# Entry function to build on Linux
# We need to make sure that the compiler is correct
#
############################################################################

# GCC compiler, mostly for linux
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_definitions(-Wall -ansi -Wno-deprecated -pthread -fmax-errors=3 -O3 -march=native -DLOG_ENABLED)
  #add_definitions(-Wall -ansi -Wno-deprecated -pthread -fmax-errors=3 -ggdb)
endif()

# Clang, mostly for Mac
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_definitions(-Wall -ansi -Wno-deprecated -O3 -march=native -DLOG_ENABLED)
endif()

# MSVC, for windows
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_definitions(/Wall /Ox -DLOG_ENABLED)
endif()

############################################################################
#
# Entry function to build on Linux
# We need to make sure that the compiler is correct
#
############################################################################

set(PYSTORM_BASE_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
set(SRC_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bddriver)
set(INC_BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../../include
                 ${CMAKE_CURRENT_SOURCE_DIR}/../../include/bddriver
)
set(LIB_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
set(INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}
    ${SRC_BASE_DIR}
    ${SRC_BASE_DIR}/comm
    ${SRC_BASE_DIR}/common
    ${SRC_BASE_DIR}/encoder
    ${SRC_BASE_DIR}/decoder
    ${SRC_BASE_DIR}/model
)

set(TEST_SRC_FILES
    ${PROJECT_SOURCE_DIR}/bddriver_app.cpp
)

include_directories(${INC_BASE_DIRS})
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${INCLUDE_DIRS})

if(CXX_IS_MSVC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PYSTORM_BASE_LIB_DIR})
endif()

add_executable(${PROJECT_NAME_STR} ${TEST_SRC_FILES})

link_directories(${PYSTORM_BASE_LIB_DIR})

target_link_libraries(${PROJECT_NAME_STR}
    ${CMAKE_THREAD_LIBS_INIT}
    Driver
)

set_property(TARGET ${PROJECT_NAME_STR} PROPERTY CXX_STANDARD ${PYSTORM_CXX_STANDARD})
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)