cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
set(TARGET pipeline)
project(${TARGET} CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wall")


set(SOURCE_FILES pipeline.cpp)
add_executable(${TARGET} ${SOURCE_FILES})
add_executable(gen_ppm gen_ppm.cpp)

target_compile_options(${TARGET} PRIVATE
  -fexceptions
)

target_link_libraries(${TARGET})
target_link_libraries(gen_ppm)
