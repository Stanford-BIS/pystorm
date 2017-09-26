# Check the build environment
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0054 NEW)
endif()

# Check for supported host
if(NOT CMAKE_HOST_SYSTEM_NAME)
    message(FATAL_ERROR "Uknown host")
endif()
if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Host type is: Linux")
elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "Host type is: Mac")
elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Host type is: Windows")
else() # Host unsupported
    message(FATAL_ERROR "Build does not support host '"
        ${CMAKE_HOST_SYSTEM_NAME} "'")
endif()

# Check for supported compilers
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Compiler ID is: GNU")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Compiler ID is: Clang")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    message(STATUS "Compiler ID is: AppleClang")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "Compiler ID is: MSVC")
else()
    message(FATAL_ERROR "Your compiler: '"
        ${CMAKE_CXX_COMPILER_ID}
        "' is not supported")
endif()
