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
    src/LinearAllocator.hpp
    src/InputSystem.hpp
    src/InputSystem.cpp

    # Memory
    src/Memory.hpp
    src/Allocator.hpp

    # Utils
    src/Defines.hpp
    src/Logger.cpp
    src/Logger.hpp
    src/Utils.hpp
    src/Utils.cpp

    # Math
    src/math/Vec3.hpp
    src/math/Mat4.hpp
    src/math/Mat4.cpp
    src/math/Float.hpp
    src/math/Quaternion.hpp
    src/math/Quaternion.cpp

    # Shaders
    src/Shader.hpp
    src/Shader.cpp

    # Rendering
    src/Renderer.hpp

    # FileSystem
    src/FileSystem.hpp

    # Gameplay
    src/PlayerInput.hpp
    
    # Textures
    src/Texture.hpp
    src/Texture.cpp

    src/main.cpp)

if(APPLE)
    target_sources(blocks PRIVATE
        src/FileSystem/Apple.cpp)
endif()

target_compile_options(blocks PRIVATE -fno-rtti -fno-exceptions -std=c++14)

target_include_directories(blocks
  PUBLIC
    deps/glad/include
    src)

target_link_libraries(blocks PUBLIC CONAN_PKG::sdl2 -lm)
