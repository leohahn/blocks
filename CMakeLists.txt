cmake_minimum_required(VERSION 3.5)

project(blocks)

if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup(TARGETS NO_OUTPUT_DIRS)
endif()

add_executable(blocks
    deps/glad/src/glad.cpp
    src/defines.h
    src/logger.cpp
    src/logger.h
    src/main.cpp)

target_compile_options(blocks PRIVATE -fno-rtti -fno-exceptions -std=c++14)

target_include_directories(blocks PUBLIC deps/glad/include)

target_link_libraries(blocks PUBLIC CONAN_PKG::sdl2)
