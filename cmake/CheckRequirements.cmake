# Exports the following variables
#
# HOST_IS_LINUX
# HOST_IS_DARWIN
# HOST_IS_WINDOWS
# HOST_IS_VALID
#
# CXX_IS_GNU
# CXX_IS_CLANG
# CXX_IS_MSVC
# CXX_IS_VALID

# Check for supported host
if(NOT CMAKE_HOST_SYSTEM_NAME)
    message(FATAL_ERROR "Uknown host")
endif()

set(HOST_IS_LINUX OFF)
set(HOST_IS_DARWIN OFF)
set(HOST_IS_WINDOWS OFF)
set(HOST_IS_VALID ON)

if (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    set(HOST_IS_LINUX ON)
    message(STATUS "Host type is: Linux")
elseif (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    set(HOST_IS_DARWIN ON)
    message(STATUS "Host type is: Mac")
elseif (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    set(HOST_IS_WINDOWS ON)
    message(STATUS "Host type is: Windows")
else()
    set(HOST_IS_VALID OFF)
endif()

# Check for supported compilers
set(CXX_IS_GNU OFF)
set(CXX_IS_CLANG OFF)
set(CXX_IS_MSVC OFF)
set(CXX_IS_VALID ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CXX_IS_GNU ON)
    message(STATUS "Compiler ID is: GNU")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CXX_IS_CLANG ON)
    message(STATUS "Compiler ID is: Clang")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CXX_IS_MSVC ON)
    message(STATUS "Compiler ID is: MSVC")
else()
    set(CXX_IS_VALID OFF)
endif()

# Check for supported compilers
if (NOT (CXX_IS_VALID))
    message(FATAL_ERROR "Your compiler: '"
        ${CMAKE_CXX_COMPILER_ID}
        "' is not supported")
endif()
