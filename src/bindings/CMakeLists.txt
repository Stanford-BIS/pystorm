cmake_minimum_required(VERSION 3.0)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)

############################################################################
#
# Global variables
#
############################################################################

project(PyStormBindings CXX)

set(TARGET_NAME "Pystorm")
set(HAL_LIBRARY Hal )
set(DRIVER_LIBRARY Driver)

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
set(HEADER_FILES "")
set(SRC_FILES "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/python/3.5)

############################################################################
#
# Check requirements that must be met for all platforms
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

#find_package(Threads REQUIRED)
find_package(Boost 1.58.0 COMPONENTS python3 REQUIRED)
find_package(PythonLibs 3.5)
find_package(PythonInterp 3.5)
find_package(Numpy)

find_library(HAL_LIBRARY Hal HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
find_library(DRIVER_LIBRARY Driver HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)

############################################################################
#
# Entry function to build on Linux
# We need to make sure that the compiler is correct
#
############################################################################

function(build_common)
    link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

    add_library(${TARGET_NAME} SHARED
        ${HEADER_FILES}
        ${SRC_FILES}
        )

    target_link_libraries(${TARGET_NAME}
        ${Boost_LIBRARIES}
        ${PYTHON_LIBRARIES}
        ${HAL_LIBRARY}
        ${DRIVER_LIBRARY}
    )

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)
    include_directories(${INCLUDE_DIRS})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/python/3.5)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../bdhal)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../bddriver)
    include_directories(${Boost_INCLUDE_DIRS})
    include_directories(${PYTHON_INCLUDE_DIRS})
    include_directories(${PYTHON_NUMPY_INCLUDE_DIR})

    set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 11)
    set_property(TARGET ${TARGET_NAME} PROPERTY PREFIX "")

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
        set(CMAKE_CXX_FLAGS "/Wall" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od -DLOG_ENABLED" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_RELEASE "/Ox -DNDEBUG" CACHE STRING "" FORCE)
    endif()
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