set(HEADER_FILES
    ${HEADER_FILES} 
    ${CMAKE_CURRENT_SOURCE_DIR}/BDPars.h
    ${CMAKE_CURRENT_SOURCE_DIR}/BDState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DriverPars.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DriverTypes.h
    # MutexBuffer.cpp is included from here:
    ${CMAKE_CURRENT_SOURCE_DIR}/MutexBuffer.h 
    # Xcoder.cpp is included from here:
    ${CMAKE_CURRENT_SOURCE_DIR}/Xcoder.h
    # binary_util.cpp is included from here
    ${CMAKE_CURRENT_SOURCE_DIR}/binary_util.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TSQueue.h
    PARENT_SCOPE
)

set(SRC_FILES
    ${SRC_FILES} 
    ${CMAKE_CURRENT_SOURCE_DIR}/BDPars.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BDState.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DriverPars.cpp
    PARENT_SCOPE
)
