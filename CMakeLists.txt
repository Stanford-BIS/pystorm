cmake_minimum_required(VERSION 3.0)
project(PyStorm CXX)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# This has to be done after 'project' is set
include(CheckRequirements)
include(Common)

# Enable testing
EnableTesting()

# Load packages
find_package(PythonLibs 3.5 REQUIRED)
find_package(PythonInterp 3.5)
find_package(Boost 1.58.0 COMPONENTS python3 REQUIRED)

############################################################################
#
# Entry function to build on Linux/Darwin/Windows
# On entry, we know that Boost is present on the system
# We need to make sure that the compiler is correct
#
############################################################################

function(build_common)
    # Build and install external dependencies into lib/ directory
    link_directories(${PROJECT_SOURCE_DIR}/lib)

    # External modules
    if (CXX_IS_GNU OR CXX_IS_CLANG)
        add_subdirectory(${PROJECT_SOURCE_DIR}/ext)
    endif()

    # PyStorm modules
    if (CXX_IS_VALID)
        add_subdirectory(${PROJECT_SOURCE_DIR}/src)
    endif()

    # Test modules
    if (CXX_IS_GNU OR CXX_IS_CLANG)
        add_subdirectory(${PROJECT_SOURCE_DIR}/test)
    endif()
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
