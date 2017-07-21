cmake_minimum_required(VERSION 3.0)

project(PyStorm CXX)

enable_testing()

############################################################################ #
# Global variables 
#
############################################################################

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
endfunction()

############################################################################
#
# Entry function to build on Linux
# On entry, we know that Boost is present on the system
# We need to make sure that the compiler is correct
#
############################################################################

function(build_on_linux)

    if (NOT CMAKE_COMPILER_IS_GNUCXX)
        message(FATAL_ERROR "CMakesLists.txt has not been tested/written for your compiler")
    endif()

    # build and install external dependencies into lib/ directory
    add_subdirectory(${PROJECT_SOURCE_DIR}/ext)     

    # Build modules
    add_subdirectory(${PROJECT_SOURCE_DIR}/src)

    # Build tests
    add_subdirectory(${PROJECT_SOURCE_DIR}/test/)                             

endfunction()

############################################################################
#
# Entry function to build on Linux and, in the future, Mac OSX and Windows
# 10
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
else()
    message(FATAL_ERROR "Build does not support system type")
endif()
