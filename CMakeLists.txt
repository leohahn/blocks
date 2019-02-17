cmake_minimum_required(VERSION 3.5)

project(blocks)

include(cmake/conan.cmake)

conan_cmake_run(REQUIRES "sdl2/2.0.9@bincrafters/stable"
                BASIC_SETUP
                TARGETS
                NO_OUTPUT_DIRS
                BUILD missing)

if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup(TARGETS NO_OUTPUT_DIRS)
endif()

add_executable(blocks
    deps/glad/src/glad.cpp
    src/linear_allocator.hpp
    src/memory.hpp
    src/allocator.hpp
    src/double_linked_list.hpp
    src/input_system.hpp
    src/input_system.cpp
    src/defines.hpp
    src/logger.cpp
    src/logger.hpp
    src/utils.hpp
    src/utils.cpp
    src/math/vec3.hpp
    src/math/mat4.hpp
    src/main.cpp)

target_compile_options(blocks PRIVATE -fno-rtti -fno-exceptions -std=c++14)

target_include_directories(blocks
  PUBLIC
    deps/glad/include
    src)

target_link_libraries(blocks PUBLIC CONAN_PKG::sdl2)
