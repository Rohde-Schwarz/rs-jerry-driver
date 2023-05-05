cmake_minimum_required(VERSION 3.14)
project(jerryDriver)

set(LOCAL_GRPC_PATH /usr)
set(ENV{PKG_CONFIG_PATH} ${LOCAL_GRPC_PATH}/share/pkgconfig:${LOCAL_GRPC_PATH}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH})
set(CMAKE_INCLUDE_PATH ${LOCAL_GRPC_PATH}/include ${CMAKE_INCLUDE_PATH})
set(CMAKE_LIBRARY_PATH ${LOCAL_GRPC_PATH}/lib ${CMAKE_LIBRARY_PATH})
set(GRPC_CPP_PLUGIN_PATH ${LOCAL_GRPC_PATH}/bin)
list(APPEND CMAKE_PREFIX_PATH ${LOCAL_GRPC_PATH})

find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DPDK REQUIRED libdpdk)

message("-- Found gRPC, version " ${gRPC_VERSION})
message("-- Found PkgConfig, version " ${PKG_CONFIG_VERSION_STRING})
message("-- Found DPDK, version " ${DPDK_VERSION})

execute_process(COMMAND pkg-config --libs --cflags protobuf grpc++ libdpdk OUTPUT_VARIABLE PKG_CONFIG_FLAGS)
add_definitions(${PKG_CONFIG_FLAGS})
add_definitions("-std=c++17")

set(CMAKE_BUILD_TYPE Release)
set(ADDITIONAL_FLAGS "-Wall -Wextra -Werror -Wno-unknown-pragmas -fPIC -O3")
set(CMAKE_CXX_FLAGS  ${CMAKE_CXX_FLAGS} ${ADDITIONAL_FLAGS})
set(CMAKE_C_FLAGS  ${CMAKE_C_FLAGS} ${ADDITIONAL_FLAGS})

OPTION(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
    enable_testing()
    include(CTest)
    add_subdirectory(gtest/dpdkSource)
    add_subdirectory(gtest/iqClient)
endif(BUILD_TESTS)

add_subdirectory(src/dpdkSource)
add_subdirectory(src/iqClient)
add_subdirectory(src/udpSource)
add_subdirectory(src/grpcClient)
add_subdirectory(src/udpTransmitter)

set_source_files_properties(${ALL_PROTO_SOURCES} PROPERTIES GENERATED TRUE)

add_library(jerryDriver STATIC ${ALL_SOURCES} ${ALL_PROTO_SOURCES})
add_dependencies(jerryDriver ${ALL_PROTO_TARGETS})
target_include_directories(jerryDriver PUBLIC ${ALL_HEADERS})
target_link_libraries(jerryDriver gRPC::grpc++ ${DPDK_LIBRARIES} jsoncpp)
include_directories(${PROJECT_SOURCE_DIR}/build/src/grpcClient/generated)

install(DIRECTORY 
    "include/iqClient"
    DESTINATION include
)

install(FILES
    ${ALL_PROTO_HEADERS}
    DESTINATION include/iqClient/generated
)

install(TARGETS jerryDriver DESTINATION lib)
