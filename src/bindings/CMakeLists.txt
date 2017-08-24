cmake_minimum_required(VERSION 3.0)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)

project(PyStormBindings CXX)

# This has to be done after 'project' is set
include(CheckRequirements)
include(Common)

############################################################################
#
# Global variables
#
############################################################################

set(TARGET_NAME "Pystorm")
set(HAL_LIBRARY Hal )
set(DRIVER_LIBRARY Driver)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
set(HEADER_FILES "")
set(SRC_FILES "")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/python/3.5)

############################################################################
#
# Additional packages
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
    # This should be before TARGET is set
    SetupSharedLibrary()

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
