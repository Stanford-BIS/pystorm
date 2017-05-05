cmake_minimum_required(VERSION 3.0)                                             
project(yaml_cpp_builder CXX)                                                    
include(ExternalProject)


option(YAML_CPP_BUILD_TOOLS_FLAG OFF)                                                 
option(SHARED_LIBRARY_FLAG ON)                                                 

set(PROJECT_BUILD_STEPS_FILE ${PROJECT_SOURCE_DIR}/project_build_steps.cmake)
                                                                                
ExternalProject_Add(yaml-cpp                                                  
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp
    CMAKE_ARGS -DBUILD_SHARED_LIBS="${SHARED_LIBRARY_FLAG}"
               -DYAML_CPP_BUILD_TOOLS="${YAML_CPP_BUILD_TOOLS_FLAG}"
    INSTALL_COMMAND cmake 
        -DBUILD_STEP=install 
        -DBASE_DIR=${PROJECT_SOURCE_DIR}
        -DBUILD_DIR=${yaml_cpp_builder_BINARY_DIR}/yaml-cpp-prefix/src/yaml-cpp-build/
        -DBUILD_INCLUDE_DIR=${yaml_cpp_builder_BINARY_DIR}/yaml-cpp-prefix/src/yaml-cpp/include/
        -DDEST_INCLUDE_DIR=${PROJECT_SOURCE_DIR}/include
        -P ${PROJECT_BUILD_STEPS_FILE}
    )
