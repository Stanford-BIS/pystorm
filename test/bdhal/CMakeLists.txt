cmake_minimum_required(VERSION 3.0)
set(PROJECT_NAME_STR bdhal_test)
project(${PROJECT_NAME_STR} CXX)

set(PYSTORM_BASE_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)

find_package(Threads REQUIRED)

if(CMAKE_COMPILER_IS_GNUCXX)
  add_definitions(-Wall -ansi -Wno-deprecated -pthread -fmax-errors=3 -O3 -march=native -DLOG_ENABLED)
  #add_definitions(-Wall -ansi -Wno-deprecated -pthread -fmax-errors=3 -ggdb)
endif()

#-------------------
# Set global variables
#-------------------
set(SRC_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bdhal) 
set(INC_BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../../include 
                 ${CMAKE_CURRENT_SOURCE_DIR}/../../include/bdhal
) 
set(LIB_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)

set(INCLUDE_DIRS 
    ${PROJECT_SOURCE_DIR}
    ${SRC_BASE_DIR}
    ${SRC_BASE_DIR}/common
)

#-------------------
# Install and build external dependencies 
#-------------------

#set(EXT_PROJECTS_DIR ${PROJECT_SOURCE_DIR}/ext)
#add_subdirectory(${EXT_PROJECTS_DIR}/gtest)

include_directories(${INC_BASE_DIRS})                                        
                                                                            
#-------------------
# Build Test
#-------------------
include_directories(${GTEST_INCLUDE_DIRS} ${INCLUDE_DIRS})

set(TEST_SRC_FILES 
    ${PROJECT_SOURCE_DIR}/common/Bucket_test.cpp
    ${PROJECT_SOURCE_DIR}/common/Connectable_test.cpp
    ${PROJECT_SOURCE_DIR}/common/Connection_test.cpp
    ${PROJECT_SOURCE_DIR}/common/Input_test.cpp
    ${PROJECT_SOURCE_DIR}/common/Network_test.cpp
    ${PROJECT_SOURCE_DIR}/common/Output_test.cpp
    ${PROJECT_SOURCE_DIR}/common/Pool_test.cpp
    ${PROJECT_SOURCE_DIR}/common/Weights_test.cpp
    ${PROJECT_SOURCE_DIR}/Hal_test.cpp
)

add_executable(${PROJECT_NAME_STR} ${TEST_SRC_FILES})

add_dependencies(${PROJECT_NAME_STR} gtest)
add_dependencies(${PROJECT_NAME_STR} gtest_main)

link_directories(${PYSTORM_BASE_LIB_DIR})
target_link_libraries(${PROJECT_NAME_STR}
    gtest
    gtest_main
    ${CMAKE_THREAD_LIBS_INIT} 
    Hal
)

set_property(TARGET ${PROJECT_NAME_STR} PROPERTY CXX_STANDARD 11)
set(CTEST_ENVIRONMENT "LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${PYSTORM_BASE_LIB_DIR}")

enable_testing()
add_test(NAME AllTests
         COMMAND bdhal_test)

set_property(TEST AllTests PROPERTY ENVIRONMENT "LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${PYSTORM_BASE_LIB_DIR}")