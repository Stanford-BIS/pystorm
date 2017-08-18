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
elseif (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    set(HOST_IS_DARWIN ON)
elseif (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    set(HOST_IS_WINDOWS ON)
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
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CXX_IS_CLANG ON)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CXX_IS_MSVC ON)
else()
    set(CXX_IS_VALID OFF)
endif()

# Check for supported compilers
if (NOT (CXX_IS_VALID))
    message(FATAL_ERROR "Your compiler: '"
        ${CMAKE_CXX_COMPILER_ID}
        "' is not supported")
endif()
