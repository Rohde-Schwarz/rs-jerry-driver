add_executable(
  iqClientTest
  iqClientTest.cc
)

target_link_libraries(
  iqClientTest
  gtest
  gtest_main
  jerryDriver
  pthread
)

include(GoogleTest)
gtest_discover_tests(iqClientTest)

target_include_directories(iqClientTest PUBLIC ${CMAKE_SOURCE_DIR}/include/)
include_directories(${PROJECT_SOURCE_DIR}/build/src/grpcClient/generated)