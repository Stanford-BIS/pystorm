cmake_minimum_required(VERSION 3.0)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)

project(PythonBindings CXX)

set(TARGET1_NAME "Pystorm")
set(TARGET2_NAME "PyDriver")
set(TARGET3_NAME "PyOK")

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

set(DRIVER_BINDINGS ${CMAKE_CURRENT_SOURCE_DIR}/python/3.5/PyDriver.cpp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../ext/pybind11 ${CMAKE_BINARY_DIR}/pybind11)
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

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(-flto -fvisibility=hidden)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-flto -fvisibility=hidden)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/GL /bigobj)
endif()

################################################################################
# Build HAL bindings
################################################################################
pybind11_add_module(${TARGET1_NAME} MODULE ${HEADER_FILES} ${SRC_FILES})
target_link_libraries(${TARGET1_NAME} PRIVATE
    ${PYTHON_LIBRARIES}
    ${HAL_LIBRARY}
    ${DRIVER_LIBRARY}
)

################################################################################
# Build Driver bindings
################################################################################
pybind11_add_module(${TARGET2_NAME} MODULE ${DRIVER_BINDINGS})
target_link_libraries(${TARGET2_NAME} PRIVATE
    ${PYTHON_LIBRARIES}
    ${DRIVER_LIBRARY}
)

set_property(TARGET ${TARGET1_NAME} ${TARGET2_NAME} PROPERTY CXX_STANDARD ${PYSTORM_CXX_STANDARD})
set_property(TARGET ${TARGET1_NAME} ${TARGET2_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${TARGET1_NAME} ${TARGET2_NAME} PROPERTY PREFIX "")

# Python expects modules to end in .so in Linux/Mac and .pyd in Windows
if(APPLE)
    set_target_properties(${TARGET1_NAME} ${TARGET2_NAME} PROPERTIES SUFFIX ".so")
elseif(WIN32)
    set_target_properties(${TARGET1_NAME} ${TARGET2_NAME} PROPERTIES SUFFIX ".pyd")
endif(APPLE)

################################################################################
# Build OpalKelly bindings
################################################################################
if(BD_COMM_TYPE MATCHES "OPALKELLY")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../FPGA/ext/opalkelly/include)
    find_library(OK_LIBRARY okFrontPanel HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../../FPGA/ext/opalkelly/lib)
    pybind11_add_module(${TARGET3_NAME} MODULE ${CMAKE_CURRENT_SOURCE_DIR}/python/3.5/PyOK.cpp)
    target_link_libraries(${TARGET3_NAME} PRIVATE
        ${OK_LIBRARY}
    )

    set_property(TARGET ${TARGET3_NAME} PROPERTY CXX_STANDARD ${PYSTORM_CXX_STANDARD})
    set_property(TARGET ${TARGET3_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
    set_property(TARGET ${TARGET3_NAME} PROPERTY PREFIX "")

    # Python expects modules to end in .so in Linux/Mac and .pyd in Windows
    if(APPLE)
        set_target_properties(${TARGET3_NAME} PROPERTIES SUFFIX ".so")
    elseif(WIN32)
        set_target_properties(${TARGET3_NAME} PROPERTIES SUFFIX ".pyd")
    endif(APPLE)
endif()
