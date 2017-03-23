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
set(SRC_BASE_DIR "../src") 
set(LIB_BASE_DIR ${PROJECT_SOURCE_DIR}/../lib)

set(INCLUDE_DIRS 
    ${SRC_BASE_DIR}
    ${SRC_BASE_DIR}/bddriver
    ${SRC_BASE_DIR}/bddriver/comm
    ${SRC_BASE_DIR}/bddriver/common
    ${SRC_BASE_DIR}/common
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
find_package(PythonLibs REQUIRED)

if(NOT Boost_FOUND)                                                         
    message(FATAL_ERROR "Unable to find correct Boost version.")        
endif()              

include_directories(${Boost_INCLUDE_DIRS} "/usr/include/python3.5")         
include_directories(${CMAKE_SOURCE_DIR}/include)                            
include_directories(${INCLUDE_DIRS})                                        
                                                                            
#-------------------
# Build Test
#-------------------
enable_testing()

set(PROJECT_TEST_NAME ${PROJECT_NAME_STR}_test)
include_directories(${GTEST_INCLUDE_DIRS} ${INCLUDE_DIRS})

set(TEST_SRC_FILES 
    ${PROJECT_SOURCE_DIR}/bddriver/comm/emulator_test.cpp
)

add_executable(${PROJECT_TEST_NAME} ${TEST_SRC_FILES})
add_dependencies(${PROJECT_TEST_NAME} googletest)


target_link_libraries(${PROJECT_TEST_NAME}
    ${GTEST_LIBS_DIR}/libgtest.a
    ${GTEST_LIBS_DIR}/libgtest_main.a
    )

target_link_libraries( ${PROJECT_TEST_NAME} ${LIB_BASE_DIR}/DriverPy )
target_link_libraries( ${PROJECT_TEST_NAME} ${CMAKE_THREAD_LIBS_INIT} )

target_link_libraries(${PROJECT_TEST_NAME}                                        
    boost_python-py35
    ${PYTHON_LIBRARIES}                                                     
    boost_system
)
#    ${Boost_LIBRARIES})            

set_property(TARGET ${PROJECT_TEST_NAME} PROPERTY CXX_STANDARD 11)
