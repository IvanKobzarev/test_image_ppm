cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
set(TARGET pipeline)
project(${TARGET} CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)


set(SOURCE_FILES pipeline.cpp)
add_executable(${TARGET} ${SOURCE_FILES})

target_compile_options(${TARGET} PRIVATE
  -fexceptions
)

add_subdirectory(third_party/gflags)

target_link_libraries(${TARGET}
  gflags::gflags
)
