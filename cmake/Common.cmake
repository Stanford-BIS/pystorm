function(SetupSharedLibrary)
    # GCC compiler, mostly for linux
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(CMAKE_CXX_FLAGS "-Wall -ansi -Wno-deprecated -pthread -fmax-errors=3" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0 -DLOG_ENABLED -gdwarf-4" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG" CACHE STRING "" FORCE)
    endif()

    # Clang, mostly for Mac
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "-Wall -Wno-deprecated" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0 -DLOG_ENABLED -gdwarf-4" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG" CACHE STRING "" FORCE)
    endif()

    # MSVC, for windows
    if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON CACHE BOOL "" FORCE)
        set(CMAKE_CXX_FLAGS "/Wall /DBOOST_ALL_NO_LIB /EHsc" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od -DLOG_ENABLED" CACHE STRING "" FORCE)
        set(CMAKE_CXX_FLAGS_RELEASE "/Ox -DNDEBUG" CACHE STRING "" FORCE)
    endif()
endfunction()

if(NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING
        "Choose the type of build, options are : Debug Release"
        FORCE)
endif()

set(PYSTORM_CXX_STANDARD 14 CACHE INTERNAL "" FORCE)
