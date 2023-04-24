file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated)

foreach(_target
  errors empty channel general ready deviceInfo login channelSettings protocols networkStreamingSettings msr4RemoteStreaming streamingSettings streamingStatus msr4RemoteStreamingFromMSR4)

    set("${_target}_proto" "${_target}.proto")
    set("${_target}_proto_srcs" "${CMAKE_CURRENT_BINARY_DIR}/generated/${_target}.pb.cc")
    set("${_target}_proto_hdrs" "${CMAKE_CURRENT_BINARY_DIR}/generated/${_target}.pb.h")
    set("${_target}_grpc_srcs" "${CMAKE_CURRENT_BINARY_DIR}/generated/${_target}.grpc.pb.cc")
    set("${_target}_grpc_hdrs" "${CMAKE_CURRENT_BINARY_DIR}/generated/${_target}.grpc.pb.h")

    add_library("${_target}" STATIC
    "${${_target}_proto_srcs}" "${${_target}_grpc_srcs}")

    add_custom_command(
        OUTPUT "${${_target}_proto_srcs}" "${${_target}_proto_hdrs}" "${${_target}_grpc_srcs}" "${${_target}_grpc_hdrs}"
        COMMAND ${GRPC_CPP_PLUGIN_PATH}/protoc
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}/generated"
            --cpp_out "${CMAKE_CURRENT_BINARY_DIR}/generated"
            -I "${PROJECT_SOURCE_DIR}/proto"
            --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_PATH}/grpc_cpp_plugin
            "${${_target}_proto}"
    )

    list(APPEND proto_sources "${${_target}_proto_srcs}" "${${_target}_grpc_srcs}")
    list(APPEND proto_headers "${${_target}_proto_hdrs}" "${${_target}_grpc_hdrs}")
    list(APPEND proto_targets "${_target}")
endforeach()

set(ALL_SOURCES ${ALL_SOURCES} ${CMAKE_CURRENT_LIST_DIR}/grpcClient.cc PARENT_SCOPE)
set(ALL_HEADERS ${ALL_HEADERS} ${CMAKE_SOURCE_DIR}/include/iqClient PARENT_SCOPE)

set(ALL_PROTO_SOURCES ${proto_sources} PARENT_SCOPE)
set(ALL_PROTO_HEADERS ${proto_headers} PARENT_SCOPE)
set(ALL_PROTO_TARGETS ${proto_targets} PARENT_SCOPE)

add_dependencies(msr4RemoteStreamingFromMSR4 streamingSettings streamingStatus ready errors empty)
add_dependencies(streamingSettings networkStreamingSettings protocols channel)
add_dependencies(general errors empty)
add_dependencies(login deviceInfo)
