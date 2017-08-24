cmake_minimum_required(VERSION 3.0)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)

project(bddriver CXX)

# This has to be done after 'project' is set
include(CheckRequirements)
include(Common)

############################################################################
#
# Global variables
#
############################################################################

set(TARGET_NAME "Driver")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
set(HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Driver.h)
set(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Driver.cpp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/comm)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/common)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/decoder)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/encoder)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/model)

############################################################################
#
# Additional packages
#
############################################################################

find_package(Threads REQUIRED)

############################################################################
#
# Entry function to build on Linux, Mac and Windows
# We need to make sure that the compiler is correct
#
############################################################################

function(build_common)
    # This should be before TARGET is set
    SetupSharedLibrary()

    # define the library being built
    add_library(${TARGET_NAME} SHARED
        ${HEADER_FILES}
        ${SRC_FILES}
        )

    target_link_libraries(${TARGET_NAME}
        ${CMAKE_THREADS_LIB_INIT})

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)
    include_directories(${INCLUDE_DIRS})

    set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 11)
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

if(HOST_IS_VALID)
    build_common()
else()
    message(FATAL_ERROR "Build does not support host '"
        ${CMAKE_HOST_SYSTEM_NAME} "'")
endif()
