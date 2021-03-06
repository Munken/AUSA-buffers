cmake_minimum_required(VERSION 2.8.4)
project(protobuf)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories("include")
include_directories("external/include")
include_directories("../AUSAlib/include")
include_directories("../AUSAlib/external/include")

include(FindROOT.cmake)
include_directories(${ROOT_INCLUDE_DIR})
link_directories(${ROOT_LIBRARY_DIR})

file(GLOB_RECURSE HEADERS "include/*.h")

file(GLOB_RECURSE SRC_
        "source/*.cpp"
        "source/*.c++"
    )

set(AUSA /home/munk/.clion10/system/cmake/generated/2875045f/2875045f/Release/libAUSA.a)

add_executable(BufTest  source/AUSAbuf.cpp)
add_library(AUSAbuf  ${SRC_} ${HEADERS})
add_library(xxhash external/source/xxhash.c)

target_link_libraries(AUSAbuf xxhash capnp kj ${AUSA} ${ROOT_LIBRARIES} lz4)
target_link_libraries(BufTest AUSAbuf)

INSTALL(TARGETS AUSAbuf
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(FILES ${HEADERS} DESTINATION include/buf)