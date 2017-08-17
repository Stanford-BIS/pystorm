cmake_minimum_required(VERSION 3.0)

project(bddriver CXX)

############################################################################
#
# Global variables
#
############################################################################

set(TARGET_NAME "Driver")

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)

set(HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Driver.h)
set(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Driver.cpp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/comm)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/common)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/decoder)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/encoder)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/model)

############################################################################
#
# Check dependencies that must be met for all platforms
#
############################################################################

if(NOT CMAKE_HOST_SYSTEM_NAME)
    message(FATAL_ERROR "system name not set")
endif()

# check pre-build requirements
if (NOT (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
            CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
            CMAKE_CXX_COMPILER_ID MATCHES "MSVC"))
    message(FATAL_ERROR "CMakesLists.txt has not been tested/written for your compiler")
endif()

############################################################################
#
# Set modules required by all platforms
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
    # Setup build environment variables
    
    # GCC compiler, mostly for linux
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(CMAKE_CXX_FLAGS "-Wall -ansi -Wno-deprecated -pthread -fmax-errors=3" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0 -DLOG_ENABLED" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG" CACHE STRING "" FORCE)
    endif()

    # Clang, mostly for Mac
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "-Wall -Wno-deprecated" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0 -DLOG_ENABLED" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG" CACHE STRING "" FORCE)
    endif()

    # MSVC, for windows
    if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
        set(CMAKE_CXX_FLAGS "/Wall" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od -DLOG_ENABLED" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_RELEASE "/Ox -DNDEBUG" CACHE STRING "" FORCE)
    endif()

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

function(build_on_linux)
    build_common()
endfunction()

function(build_on_mac)
    build_common()
endfunction()

function(build_on_windows)
    build_common()
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

if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
    build_on_linux()
elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin")
    build_on_mac()
elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    build_on_windows()
else()
    message(FATAL_ERROR "Build does not support system type")
endif()