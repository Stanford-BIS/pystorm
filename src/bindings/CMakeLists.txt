cmake_minimum_required(VERSION 3.0)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)

set(TARGET1_NAME "_PyStorm")
set(TARGET2_NAME "_PyDriver")
set(TARGET3_NAME "_PyOK")

############################################################################
#
# Common setup and sanity checks
#
############################################################################
set(PRJ_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(BD_LIBRARY_OUTPUT_PATH ${PRJ_ROOT_DIR}/lib)
list(APPEND CMAKE_MODULE_PATH ${PRJ_ROOT_DIR}/cmake)
# This has to be done after 'project' is set
include(CheckRequirements)
include(Common)

# This should be before TARGET is set
SetupCompilerOptions()
SetupBuildPaths()

############################################################################
#
# Build setup
#
############################################################################

if($<CONFIG> STREQUAL "Release")
    set(PYSTORM_BASE_LIB_DIR ${BD_LIBRARY_OUTPUT_PATH}/Release)
elseif($<CONFIG> STREQUAL "Debug")
    set(PYSTORM_BASE_LIB_DIR ${BD_LIBRARY_OUTPUT_PATH}/Debug)
endif()

set(HAL_LIBRARY Hal )
set(DRIVER_LIBRARY Driver)
find_library(HAL_LIBRARY Hal HINTS ${BD_LIBRARY_OUTPUT_PATH}/$<CONFIG>)
find_library(DRIVER_LIBRARY Driver HINTS ${BD_LIBRARY_OUTPUT_PATH}/$<CONFIG>)

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
set(HEADER_FILES "")
set(SRC_FILES "")

set(DRIVER_BINDINGS ${CMAKE_CURRENT_SOURCE_DIR}/python/3.5/PyDriver/PyDriver.cpp)

add_subdirectory(${PRJ_ROOT_DIR}/ext/pybind11 ${CMAKE_BINARY_DIR}/pybind11)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/python/3.5)

include_directories(${PRJ_ROOT_DIR}/include)
include_directories(${INCLUDE_DIRS})
include_directories(${PRJ_ROOT_DIR}/ext/pybind11/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/python/3.5)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../bdhal)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../bddriver)

link_directories(${PYSTORM_BASE_LIB_DIR})

################################################################################
# Build HAL bindings
################################################################################
pybind11_add_module(${TARGET1_NAME} MODULE ${HEADER_FILES} ${SRC_FILES})
target_link_libraries(${TARGET1_NAME} PRIVATE
    ${HAL_LIBRARY}
    ${DRIVER_LIBRARY}
)

# copy or link bindings library into pystorm source tree
if(Win32)
    add_custom_command(
        TARGET ${TARGET1_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:${TARGET1_NAME}>
            ${CMAKE_CURRENT_SOURCE_DIR}/../../pystorm/
    )
else()
    add_custom_command(
        TARGET ${TARGET1_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            $<TARGET_FILE:${TARGET1_NAME}>
            ${CMAKE_CURRENT_SOURCE_DIR}/../../pystorm/$<TARGET_FILE_NAME:${TARGET1_NAME}>
    )
endif()

#################################################################################
## Build Driver bindings
#################################################################################
pybind11_add_module(${TARGET2_NAME} MODULE ${DRIVER_BINDINGS})
target_link_libraries(${TARGET2_NAME} PRIVATE
    ${DRIVER_LIBRARY}
)

set_property(TARGET ${TARGET2_NAME} PROPERTY ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PRJ_ROOT_DIR}/lib/Debug/PyDriver)
set_property(TARGET ${TARGET2_NAME} PROPERTY ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PRJ_ROOT_DIR}/lib/Release/PyDriver)
set_property(TARGET ${TARGET2_NAME} PROPERTY LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PRJ_ROOT_DIR}/lib/Debug/PyDriver)
set_property(TARGET ${TARGET2_NAME} PROPERTY LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PRJ_ROOT_DIR}/lib/Release/PyDriver)
set_property(TARGET ${TARGET2_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PRJ_ROOT_DIR}/lib/Debug/PyDriver)
set_property(TARGET ${TARGET2_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PRJ_ROOT_DIR}/lib/Release/PyDriver)

add_custom_command(
    TARGET ${TARGET2_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/python/3.5/PyDriver/__init__.py
    ${PRJ_ROOT_DIR}/lib/$<CONFIG>/PyDriver/__init__.py
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

# copy or link bindings library into pystorm source tree
if(Win32)
    add_custom_command(
        TARGET ${TARGET2_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/$<CONFIG>/PyDriver
        ${CMAKE_CURRENT_SOURCE_DIR}/../../pystorm/
    )
else()
    add_custom_command(
        TARGET ${TARGET2_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/$<CONFIG>/PyDriver
            ${CMAKE_CURRENT_SOURCE_DIR}/../../pystorm/PyDriver
    )
endif()

################################################################################
# Build OpalKelly bindings
################################################################################
if(BD_COMM_TYPE STREQUAL "OPALKELLY")
    include_directories(${PRJ_ROOT_DIR}/FPGA/ext/opalkelly/include)
    find_library(OK_LIBRARY okFrontPanel HINTS ${PRJ_ROOT_DIR}/FPGA/ext/opalkelly/lib)
    pybind11_add_module(${TARGET3_NAME} MODULE ${CMAKE_CURRENT_SOURCE_DIR}/python/3.5/PyOK/PyOK.cpp)
    target_link_libraries(${TARGET3_NAME} PRIVATE
        ${OK_LIBRARY}
    )

    set_property(TARGET ${TARGET3_NAME} PROPERTY ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PRJ_ROOT_DIR}/lib/Debug/PyOK)
    set_property(TARGET ${TARGET3_NAME} PROPERTY ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PRJ_ROOT_DIR}/lib/Release/PyOK)
    set_property(TARGET ${TARGET3_NAME} PROPERTY LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PRJ_ROOT_DIR}/lib/Debug/PyOK)
    set_property(TARGET ${TARGET3_NAME} PROPERTY LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PRJ_ROOT_DIR}/lib/Release/PyOK)
    set_property(TARGET ${TARGET3_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PRJ_ROOT_DIR}/lib/Debug/PyOK)
    set_property(TARGET ${TARGET3_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PRJ_ROOT_DIR}/lib/Release/PyOK)

    add_custom_command(
        TARGET ${TARGET3_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/python/3.5/PyOK/__init__.py
        ${PRJ_ROOT_DIR}/lib/$<CONFIG>/PyOK/__init__.py
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
