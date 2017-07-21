cmake_minimum_required(VERSION 3.0)

project(PyStormBindings CXX)

############################################################################
#
# Global variables 
#
############################################################################

set(TARGET_NAME "Pystorm")

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)

set(HEADER_FILES "")
set(SRC_FILES "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/python/3.5)

############################################################################
#
# Check dependencies that must be met for all platforms 
#
############################################################################

function(check_multiplatform_system_dependencies)
    #find_package(Threads REQUIRED)
    find_package(Boost 1.58.0 COMPONENTS python3 REQUIRED)
    find_package(PythonLibs 3.5)
    find_package(PythonInterp 3.5)

    if(NOT CMAKE_HOST_SYSTEM_NAME)
        message(FATAL_ERROR "system name not set")
    endif()
endfunction()

############################################################################
#
# Set properties common to all platforms
#
############################################################################

function(set_common_properties target_name)
    find_package(Boost 1.58.0 COMPONENTS python3 REQUIRED)
    find_package(PythonLibs 3.5 REQUIRED)
    find_package(PythonInterp 3.5)

    find_library(HAL_LIBRARY Hal HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
    find_library(DRIVER_LIBRARY Driver HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)

    #check_minimum_python_version(${PYTHONLIBS_VERSION_STRING})

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)
    include_directories(${INCLUDE_DIRS})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/python/3.5)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../bdhal)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../bddriver)
    include_directories(${Boost_INCLUDE_DIRS})
    include_directories(${PYTHON_INCLUDE_DIRS})

    set_property(TARGET ${target_name} PROPERTY CXX_STANDARD 11)
    set_property(TARGET ${target_name} PROPERTY PREFIX "")
endfunction()

############################################################################
#
# Entry function to build on Linux
# We need to make sure that the compiler is correct
#
############################################################################

function(build_on_linux)

    # check pre-build requirements

    if (NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        message(FATAL_ERROR "CMakesLists.txt has not been tested/written for your compiler")
    endif()

    find_package(Boost 1.58.0 COMPONENTS python3 REQUIRED)
    find_package(PythonLibs 3.5)
    find_package(PythonInterp 3.5)

    find_library(HAL_LIBRARY Hal HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
    find_library(DRIVER_LIBRARY Driver HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)

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

    set_common_properties(${TARGET_NAME})

    if(CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS "-Wall -ansi -Wno-deprecated -pthread -fmax-errors=3" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0 -DLOG_ENABLED" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG" CACHE STRING "" FORCE)
    endif()

endfunction()

function(build_on_mac)
    # check pre-build requirements

    if (NOT (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID
        MATCHES "Clang"))
        message(FATAL_ERROR "CMakesLists.txt has not been tested/written for your compiler")
    endif()

    # setup build environment variables

    # define the library being built

    add_library(${TARGET_NAME} SHARED 
        ${HEADER_FILES}
        ${SRC_FILES}
        )

    # set the properties related to the target 
    # this is a function common to all platforms

    set_common_properties(${TARGET_NAME})

    if(CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS "-Wall -ansi -Wno-deprecated -pthread -fmax-errors=3" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0 -DLOG_ENABLED" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG" CACHE STRING "" FORCE)
    elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")
        set(CMAKE_CXX_FLAGS "-Wall -Wno-deprecated" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0 -DLOG_ENABLED" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG" CACHE STRING "" FORCE)
    endif()

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
elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin")
    build_on_mac()
else()
    message(FATAL_ERROR "Build does not support system type")
endif()
