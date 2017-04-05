cmake_minimum_required(VERSION 3.0)
set(PROJECT_NAME_STR bddriver)
project(${PROJECT_NAME_STR} C CXX)

find_package(Threads REQUIRED)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-Wall -ansi -Wno-deprecated -pthread)
endif()

#-------------------
# Set global variables
#-------------------
set(SRC_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src) 
set(INC_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include) 
set(LIB_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lib)

set(INCLUDE_DIRS 
    ${SRC_BASE_DIR}
    ${SRC_BASE_DIR}/comm
    ${SRC_BASE_DIR}/common
    ${SRC_BASE_DIR}/encoder
)

#-------------------
# Install and build external dependencies 
#-------------------

set(EXT_PROJECTS_DIR ${PROJECT_SOURCE_DIR}/ext)
add_subdirectory(${EXT_PROJECTS_DIR}/gtest)

#-------------------
# Install and build external dependencies 
#-------------------

find_package(Boost 1.58.0 COMPONENTS python-py35 REQUIRED)
find_package(PythonLibs 3.5 REQUIRED)

if(NOT Boost_FOUND)                                                         
    message(FATAL_ERROR "Unable to find correct Boost version.")        
endif()              

include_directories(${Boost_INCLUDE_DIRS} "/usr/include/python3.5")         
include_directories(${INC_BASE_DIR})                                        
                                                                            
#-------------------
# Build Test
#-------------------
enable_testing()

set(PROJECT_TEST_NAME ${PROJECT_NAME_STR}_test)
include_directories(${GTEST_INCLUDE_DIRS} ${INCLUDE_DIRS})

set(TEST_SRC_FILES 
    ${PROJECT_SOURCE_DIR}/comm/emulator_test.cpp
    ${PROJECT_SOURCE_DIR}/common/MutexBuffer_test.cpp
    ${PROJECT_SOURCE_DIR}/common/Binary_test.cpp
    ${PROJECT_SOURCE_DIR}/encoder/Encoder_test.cpp
)

add_executable(${PROJECT_TEST_NAME} ${TEST_SRC_FILES})

add_dependencies(${PROJECT_TEST_NAME} googletest)


target_link_libraries(${PROJECT_TEST_NAME}
    ${GTEST_LIBS_DIR}/libgtest.a
    ${GTEST_LIBS_DIR}/libgtest_main.a
    ${CMAKE_THREAD_LIBS_INIT} 
    ${LIB_BASE_DIR}/DriverPy 
    boost_python-py35
    ${PYTHON_LIBRARIES}                                                     
    boost_system
)


set_property(TARGET ${PROJECT_TEST_NAME} PROPERTY CXX_STANDARD 11)
