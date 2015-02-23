cmake_minimum_required(VERSION 2.8.4)
project(protobuf)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/home/munk/AUSA/protobuf")

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS addressbook.proto test.proto)




add_executable(write "main.cpp" ${PROTO_SRCS} ${PROTO_HDRS})
add_executable(read  "read.cpp" ${PROTO_SRCS} ${PROTO_HDRS})
add_executable(capn  "capn.cpp" "test.capn.h" "test.capn.c++")

target_link_libraries(read ${PROTOBUF_LIBRARIES})
target_link_libraries(write ${PROTOBUF_LIBRARIES})
target_link_libraries(capn capnp kj)