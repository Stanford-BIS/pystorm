cmake_minimum_required(VERSION 3.0)

project(PyStorm CXX)

############################################################################ #
# Global variables 
#
############################################################################

enable_testing()

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
    find_package(Boost 1.58.0 COMPONENTS python3 REQUIRED)
    find_package(PythonLibs REQUIRED)
    check_minimum_python_version(${PYTHONLIBS_VERSION_STRING})
    find_package(Threads REQUIRED)                                                  

    if(NOT CMAKE_HOST_SYSTEM_NAME)
        message(FATAL_ERROR "system name not set")
    endif()
endfunction()


function(build_pystorm_bindings_lib_on_linux)
    set(PYSTORM_LIB_TARGET_NAME "PyStorm")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)       

    set(PYSTORM_LIB_HEADER_FILES "")
    set(PYSTORM_LIB_SRC_FILES "")
    add_subdirectory(${PROJECT_SOURCE_DIR}/bindings/python/3.5/src)

    find_package(PythonLibs REQUIRED)
    find_package(Boost 1.58.0 COMPONENTS python3 REQUIRED)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/bindings/python/3.5/src)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/bdhal)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/bddriver)
    include_directories(${Boost_INCLUDE_DIRS})
    include_directories(${PYTHON_INCLUDE_DIRS})

    add_library(${PYSTORM_LIB_TARGET_NAME} SHARED                                           
        ${PYSTORM_LIB_HEADER_FILES}                                                         
        ${PYSTORM_LIB_SRC_FILES}                                                            
    )                                                                       

    target_link_libraries(${PYSTORM_LIB_TARGET_NAME} 
        ${Boost_LIBRARIES}
        ${PYTHON_LIBRARIES}
        Hal
        Driver
    )

    set_property(TARGET ${PYSTORM_LIB_TARGET_NAME} PROPERTY CXX_STANDARD 11)                
    set_property(TARGET ${PYSTORM_LIB_TARGET_NAME} PROPERTY PREFIX "")
                                                                                
    # set the properties related to the target                                  
    # this is a function common to all platforms                                
                                                                                
    if(CMAKE_COMPILER_IS_GNUCXX)                                                
        set(CMAKE_CXX_FLAGS "-Wall -ansi -Wno-deprecated -pthread -fmax-errors=3" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0 -DLOG_ENABLED" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG" CACHE STRING "" FORCE)
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

    link_directories(${PROJECT_SOURCE_DIR}/lib)

    # build and install external dependencies into lib/ directory
    add_subdirectory(${PROJECT_SOURCE_DIR}/ext)     

    # Build modules
    add_subdirectory(${PROJECT_SOURCE_DIR}/src)

    # Build tests
    add_subdirectory(${PROJECT_SOURCE_DIR}/test/)                             

    build_pystorm_bindings_lib_on_linux()

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
