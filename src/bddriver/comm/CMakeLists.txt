# Tests uses SOFT and MODEL. Hence need both irrespective of actual type

set(FILT_HEADERS "")
if(BD_COMM_TYPE MATCHES OPALKELLY)
set(FILT_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/CommOK.h
)
elseif(BD_COMM_TYPE MATCHES USB)
set(FILT_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/CommUSB.h
)
endif()

set(FILT_SRC "")
if(BD_COMM_TYPE MATCHES OPALKELLY)
set(FILT_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/CommOK.cpp
)
elseif(BD_COMM_TYPE MATCHES USB)
set(FILT_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/CommUSB.cpp
)
endif()

set(HEADER_FILES
    ${HEADER_FILES} 
    ${CMAKE_CURRENT_SOURCE_DIR}/Comm.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CommBDModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CommSoft.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Emulator.h
    ${FILT_HEADERS}
    PARENT_SCOPE
)

set(SRC_FILES
    ${SRC_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/CommBDModel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CommSoft.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Emulator.cpp
    ${FILT_SRC}
    PARENT_SCOPE
    )

