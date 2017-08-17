cmake_minimum_required(VERSION 3.0)

project(PyStorm CXX)

enable_testing()

############################################################################ #
# Global variables
#
############################################################################

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
    CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    enable_testing()
endif()

############################################################################
#
# Check minimum python version (python 3) is installed
#
############################################################################

function(check_minimum_python_version python_version)
    if(NOT (${python_version} MATCHES "^3.*$"))
        message(FATAL_ERROR "Python 3 not found")
    endif()
endfunction()

############################################################################
#
# Check dependencies that must be met for all platforms
#
############################################################################

function(check_multiplatform_system_dependencies)
    find_package(PythonLibs 3.5 REQUIRED)
    find_package(PythonInterp 3.5)
    find_package(Boost 1.58.0 COMPONENTS python3 REQUIRED)

    if(NOT CMAKE_HOST_SYSTEM_NAME)
        message(FATAL_ERROR "system name not set")
    endif()

    if (NOT (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
             CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
             CMAKE_CXX_COMPILER_ID MATCHES "MSVC"))
        message(FATAL_ERROR "CMakesLists.txt has not been tested/written for your compiler")
    endif()
endfunction()

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
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
        CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_subdirectory(${PROJECT_SOURCE_DIR}/ext)
    endif()

    # PyStorm modules
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
        CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
        CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        add_subdirectory(${PROJECT_SOURCE_DIR}/src)
    endif()

    # Test modules
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
        CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_subdirectory(${PROJECT_SOURCE_DIR}/test/)
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

check_multiplatform_system_dependencies()

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
