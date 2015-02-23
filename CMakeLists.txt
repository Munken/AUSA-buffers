cmake_minimum_required(VERSION 2.8.4)
project(protobuf)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/home/munk/AUSA/protobuf")

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories("include")
include_directories("../AUSAlib/include")
include_directories("../AUSAlib/external/include")

include_directories(${CMAKE_CURRENT_BINARY_DIR})
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS addressbook.proto test.proto)


include(FindROOT.cmake)
include_directories(${ROOT_INCLUDE_DIR})
link_directories(${ROOT_LIBRARY_DIR})


file(GLOB_RECURSE SRC_
        "include/*.h"
        "source/*.cpp"
        "source/*.c++"
    )

set(AUSA /home/munk/.clion10/system/cmake/generated/2875045f/2875045f/RelWithDebInfo/libAUSA.a)

add_executable(AUSAbuf  ${SRC_})
target_link_libraries(AUSAbuf capnp kj)
target_link_libraries(AUSAbuf ${AUSA})
target_link_libraries(AUSAbuf ${ROOT_LIBRARIES})

add_executable(write "main.cpp" ${PROTO_SRCS} ${PROTO_HDRS})
add_executable(read  "read.cpp" ${PROTO_SRCS} ${PROTO_HDRS})
add_executable(capn  "capn.cpp" "test.capn.h" "test.capn.c++")

target_link_libraries(read ${PROTOBUF_LIBRARIES})
target_link_libraries(write ${PROTOBUF_LIBRARIES})
target_link_libraries(capn capnp kj)
