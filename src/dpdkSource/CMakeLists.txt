set(ALL_SOURCES ${ALL_SOURCES} 
    ${CMAKE_CURRENT_LIST_DIR}/dpdkSource.cc
    ${CMAKE_CURRENT_LIST_DIR}/hrzrHeaderParser.cc
    ${CMAKE_CURRENT_LIST_DIR}/hrzrParser.cc
    ${CMAKE_CURRENT_LIST_DIR}/hrzrMetadataParser.cc
    PARENT_SCOPE)
set(ALL_HEADERS ${ALL_HEADERS}
    ${CMAKE_SOURCE_DIR}/include/iqClient
    ${CMAKE_SOURCE_DIR}/include/iqClient/dpdkSource
    PARENT_SCOPE)