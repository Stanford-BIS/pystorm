# Set variables used commonly throughout build
# Defines:
#   CMAKE_CXX_FLAGS
#   CMAKE_CXX_FLAGS_DEBUG
#   CMAKE_CXX_FLAGS_RELEASE
#   CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS
#   CMAKE_CONFIGURATION_TYPES
#   PYSTORM_CXX_STANDARD

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0054 NEW)
endif()

function(SetupCompilerOptions)
    # GCC compiler, mostly for linux
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "-Wall -ansi -Wno-deprecated -pthread -fmax-errors=3 ${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0 -DLOG_ENABLED -gdwarf-4" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG" CACHE STRING "" FORCE)
    endif()

    # Clang, mostly for Mac
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        set(CMAKE_CXX_FLAGS "-Wall -Wno-deprecated ${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0 -DLOG_ENABLED -gdwarf-4" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG" CACHE STRING "" FORCE)
    endif()

    # MSVC, for windows
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON CACHE BOOL "" FORCE)
        set(CMAKE_CXX_FLAGS "/Wall /DBOOST_ALL_NO_LIB /EHsc ${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od -DLOG_ENABLED" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_RELEASE "/Ox -DNDEBUG" CACHE STRING "" FORCE)
    endif()
endfunction()

if(NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING
        "Build types for multi-configuration setup are: Release, Debug"
        FORCE)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Specify build type for single-configuration setup from one of: Release, Debug"
        FORCE)
endif()

set(PYSTORM_CXX_STANDARD 14 CACHE INTERNAL "" FORCE)

function(SetupBuildPaths)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BD_LIBRARY_OUTPUT_PATH} CACHE STRING "" FORCE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${BD_LIBRARY_OUTPUT_PATH}/Release CACHE STRING "" FORCE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${BD_LIBRARY_OUTPUT_PATH}/Debug CACHE STRING "" FORCE)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BD_LIBRARY_OUTPUT_PATH} CACHE STRING "" FORCE)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${BD_LIBRARY_OUTPUT_PATH}/Release CACHE STRING "" FORCE)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${BD_LIBRARY_OUTPUT_PATH}/Debug CACHE STRING "" FORCE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BD_LIBRARY_OUTPUT_PATH} CACHE STRING "" FORCE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BD_LIBRARY_OUTPUT_PATH}/Release CACHE STRING "" FORCE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BD_LIBRARY_OUTPUT_PATH}/Debug CACHE STRING "" FORCE)
endfunction()