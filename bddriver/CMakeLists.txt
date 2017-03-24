cmake_minimum_required(VERSION 3.0)

project(bddriver CXX)

############################################################################
#
# Global variables 
#
############################################################################

set(TARGET_NAME "DriverPy")

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/comm)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/common)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/decoder)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/driverifc)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/encoder)

############################################################################
#
# Check dependencies that must be met for all platforms 
#
############################################################################
function(check_multiplatform_system_dependencies)
    # Boost python must 
    find_package(Boost 1.58.0 COMPONENTS python-py35)

    if(NOT Boost_FOUND)
            message(FATAL_ERROR "Unable to find correct Boost version.")
    endif()

    find_package(PythonLibs REQUIRED)
    
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
    target_link_libraries(${target_name} 
        boost_python-py35
        ${PYTHON_LIBRARIES}
        ${Boost_LIBRARIES})

    set_property(TARGET ${target_name} PROPERTY CXX_STANDARD 11)
    set_property(TARGET ${target_name} PROPERTY PREFIX "")
endfunction()


############################################################################
#
# Entry function to build on Linux
# On entry, we know that Boost is present on the system
# We need to make sure that the compiler is correct
#
############################################################################

function(build_on_linux)

    # check pre-build requirements

    if (NOT CMAKE_COMPILER_IS_GNUCXX)
        message(FATAL_ERROR "CMakesLists.txt has not been tested/written for your compiler")
    endif()

    # setup build environment variables

    include_directories(${Boost_INCLUDE_DIRS} "/usr/include/python3.5")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
    include_directories(${INCLUDE_DIRS})

    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTHREADED ON)
    set(Boost_USE_STATIC_RUNTIME ON)

    # define the library being built

    add_library(${TARGET_NAME} SHARED 
        ${HEADER_FILES}
        ${SRC_FILES}
        )

    # set the properties related to the target 
    # this is a function common to all platforms

    set_common_properties(${TARGET_NAME})

endfunction()

############################################################################
#
# Entry function to build on Linux and, in the future, Mac OSX and Windows
# 10
#
############################################################################

check_multiplatform_system_dependencies()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
    build_on_linux()
else()
    message(FATAL_ERROR "Build does not support system type")
endif()
