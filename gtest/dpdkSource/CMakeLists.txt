add_executable(
  dpdkSourceTest
  hrzrHeaderParserTest.cc
  hrzrParserTest.cc
  hrzrMetadataTest.cc
)

target_link_libraries(
  dpdkSourceTest
  gtest
  gtest_main
  pthread
  jerryDriver
)

include(GoogleTest)
gtest_discover_tests(dpdkSourceTest)

target_include_directories(dpdkSourceTest PUBLIC ${CMAKE_SOURCE_DIR}/include/iqClient)
configure_file(controltypes.bin ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(controltypesWithTimestamp.bin ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(controltypesReverseSequenceNumber.bin ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(floatsamples.in ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(floatsamples.norm ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(metadata_time_clock_types.bin ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(header_metadata.bin ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
