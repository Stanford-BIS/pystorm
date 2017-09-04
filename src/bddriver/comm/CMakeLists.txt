set(HEADER_FILES
    ${HEADER_FILES} 
    ${CMAKE_CURRENT_SOURCE_DIR}/Comm.h
    PARENT_SCOPE
)

if(BD_COMM_TYPE MATCHES "MODEL")
    set(HEADER_FILES
        ${HEADER_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/CommBDModel.h
        PARENT_SCOPE
        )
elseif(BD_COMM_TYPE MATCHES "SOFT")
    set(HEADER_FILES
        ${HEADER_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/CommSoft.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Emulator.h
        PARENT_SCOPE
        )
elseif(BD_COMM_TYPE MATCHES "USB")
    set(HEADER_FILES
        ${HEADER_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/CommUSB.h
        PARENT_SCOPE
        )
elseif(BD_COMM_TYPE MATCHES "OPALKELLY")
    set(HEADER_FILES
        ${HEADER_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/CommOK.h
        PARENT_SCOPE
        )
endif()

if(BD_COMM_TYPE MATCHES "MODEL")
    set(SRC_FILES
        ${SRC_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/CommBDModel.cpp
        PARENT_SCOPE
        )
elseif(BD_COMM_TYPE MATCHES "SOFT")
    set(SRC_FILES
        ${SRC_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/CommSoft.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Emulator.cpp
        PARENT_SCOPE
        )
elseif(BD_COMM_TYPE MATCHES "USB")
    set(SRC_FILES
        ${SRC_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/CommUSB.cpp
        PARENT_SCOPE
        )
elseif(BD_COMM_TYPE MATCHES "OPALKELLY")
    set(SRC_FILES
        ${SRC_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/CommOK.cpp
        PARENT_SCOPE
        )
endif()