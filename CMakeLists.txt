cmake_minimum_required(VERSION 2.8.4)
project(protobuf)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/home/munk/AUSA/ausa-buffers")

include_directories("include")
include_directories("external/include")
include_directories("../AUSAlib/include")
include_directories("../AUSAlib/external/include")

include(FindROOT.cmake)
include_directories(${ROOT_INCLUDE_DIR})
link_directories(${ROOT_LIBRARY_DIR})

file(GLOB_RECURSE SRC_
        "include/*.h"
        "source/*.cpp"
        "source/*.c++"
    )

set(AUSA /home/munk/.clion10/system/cmake/generated/2875045f/2875045f/Release/libAUSA.a)

add_executable(BufTest  source/AUSAbuf.cpp)
add_library(AUSAbuf  ${SRC_})
add_library(xxhash external/source/xxhash.c)

target_link_libraries(AUSAbuf xxhash capnp kj ${AUSA} ${ROOT_LIBRARIES} lz4)
target_link_libraries(BufTest AUSAbuf)
