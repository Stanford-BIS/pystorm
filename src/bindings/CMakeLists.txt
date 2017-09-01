cmake_minimum_required(VERSION 3.0)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)

project(PyStormBindings CXX)
set(TARGET_NAME "Pystorm")

############################################################################
#
# Common setup and sanity checks
#
############################################################################

# This has to be done after 'project' is set
include(CheckRequirements)
include(Common)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
    "Choose the type of build, options are : Debug Release"
    FORCE)
endif()

if(NOT HOST_IS_VALID)
    message(FATAL_ERROR "Build does not support host '"
            ${CMAKE_HOST_SYSTEM_NAME} "'")
endif()

############################################################################
#
# Build setup
#
############################################################################

# Additional packages
#find_package(Threads REQUIRED)
find_package(PythonLibs 3.5)
find_package(PythonInterp 3.5)
find_package(Numpy)

set(HAL_LIBRARY Hal )
set(DRIVER_LIBRARY Driver)
find_library(HAL_LIBRARY Hal HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
find_library(DRIVER_LIBRARY Driver HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)

# Project settings
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
if(CXX_IS_MSVC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
endif()

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
set(HEADER_FILES "")
set(SRC_FILES "")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../ext/pybind11 ${CMAKE_CURRENT_SOURCE_DIR}/python)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/python/3.5)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)
include_directories(${INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../ext/pybind11/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/python/3.5)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../bdhal)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../bddriver)
include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(${PYTHON_NUMPY_INCLUDE_DIR})

# This should be before TARGET is set
SetupSharedLibrary()

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

add_library(${TARGET_NAME} MODULE ${HEADER_FILES} ${SRC_FILES})
target_link_libraries(${TARGET_NAME}
    pybind11::module
    ${PYTHON_LIBRARIES}
    ${HAL_LIBRARY}
    ${DRIVER_LIBRARY}
)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(-flto -fvisibility=hidden)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-flto -fvisibility=hidden)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/GL /bigobj)
endif()

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${TARGET_NAME} PROPERTY PREFIX "")
# Python expects modules to end in .so
if(APPLE)
    set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".so")
elseif(WIN32)
    set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".pyd" LINK_FLAGS "/LTCG")
endif(APPLE)