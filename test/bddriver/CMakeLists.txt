cmake_minimum_required(VERSION 3.0)
set(PROJECT_NAME_STR bddriver_test)
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
set(SRC_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bddriver) 
set(INC_BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../../include 
                 ${CMAKE_CURRENT_SOURCE_DIR}/../../include/bddriver
) 
set(LIB_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)

set(INCLUDE_DIRS 
    ${PROJECT_SOURCE_DIR}
    ${SRC_BASE_DIR}
    ${SRC_BASE_DIR}/comm
    ${SRC_BASE_DIR}/common
    ${SRC_BASE_DIR}/encoder
    ${SRC_BASE_DIR}/decoder
    ${SRC_BASE_DIR}/model
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
    ${PROJECT_SOURCE_DIR}/test_util/DriverTypes_util.cpp
    ${PROJECT_SOURCE_DIR}/comm/Emulator_test.cpp
    ${PROJECT_SOURCE_DIR}/comm/CommSoft_test.cpp
    ${PROJECT_SOURCE_DIR}/common/MutexBuffer_test.cpp
    ${PROJECT_SOURCE_DIR}/encoder/Encoder_test.cpp
    ${PROJECT_SOURCE_DIR}/decoder/Decoder_test.cpp
    ${PROJECT_SOURCE_DIR}/common/Binary_test.cpp
    ${PROJECT_SOURCE_DIR}/common/DriverTypes_test.cpp
    ${PROJECT_SOURCE_DIR}/common/BDPars_test.cpp
    ${PROJECT_SOURCE_DIR}/common/DriverPars_test.cpp
    ${PROJECT_SOURCE_DIR}/common/BDState_test.cpp
    ${PROJECT_SOURCE_DIR}/Driver_test.cpp
    ${PROJECT_SOURCE_DIR}/include/logger_test.cpp
)

add_executable(${PROJECT_NAME_STR} ${TEST_SRC_FILES})

#add_dependencies(${PROJECT_NAME_STR} gtest)

target_link_libraries(${PROJECT_NAME_STR}
    ${PYSTORM_BASE_LIB_DIR}/libgtest.a
    ${PYSTORM_BASE_LIB_DIR}/libgtest_main.a
    ${CMAKE_THREAD_LIBS_INIT} 
    Driver
)

set_property(TARGET ${PROJECT_NAME_STR} PROPERTY CXX_STANDARD 11)
set(CTEST_ENVIRONMENT "LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${PYSTORM_BASE_LIB_DIR}")

enable_testing()
add_test(NAME AllTests
         COMMAND bddriver_test)

set_property(TEST AllTests PROPERTY ENVIRONMENT "LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${PYSTORM_BASE_LIB_DIR}")
