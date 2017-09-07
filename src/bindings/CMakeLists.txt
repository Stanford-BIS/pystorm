cmake_minimum_required(VERSION 3.0)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)

project(PyStormBindings CXX)
set(TARGET_NAME "Pystorm")
set(TARGET2_NAME "BDDriver")

############################################################################
#
# Common setup and sanity checks
#
############################################################################

# This has to be done after 'project' is set
include(CheckRequirements)
include(Common)

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

pybind11_add_module(${TARGET_NAME} MODULE ${HEADER_FILES} ${SRC_FILES})
target_link_libraries(${TARGET_NAME} PRIVATE
    ${PYTHON_LIBRARIES}
    ${HAL_LIBRARY}
    ${DRIVER_LIBRARY}
)
pybind11_add_module(${TARGET2_NAME} MODULE ${CMAKE_CURRENT_SOURCE_DIR}/python/3.5/BDDriver.cpp)
target_link_libraries(${TARGET2_NAME} PRIVATE
    ${PYTHON_LIBRARIES}
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

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD ${PYSTORM_CXX_STANDARD})
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${TARGET_NAME} PROPERTY PREFIX "")
# Python expects modules to end in .so in Linux/Mac and .pyd in Windows
if(APPLE)
    set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".so")
elseif(WIN32)
    set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".pyd")
endif(APPLE)

set_property(TARGET ${TARGET2_NAME} PROPERTY CXX_STANDARD ${PYSTORM_CXX_STANDARD})
set_property(TARGET ${TARGET2_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${TARGET2_NAME} PROPERTY PREFIX "")
# Python expects modules to end in .so in Linux/Mac and .pyd in Windows
if(APPLE)
    set_target_properties(${TARGET2_NAME} PROPERTIES SUFFIX ".so")
elseif(WIN32)
    set_target_properties(${TARGET2_NAME} PROPERTIES SUFFIX ".pyd")
endif(APPLE)
