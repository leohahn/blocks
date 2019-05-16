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

# Copy the resources folder to the correct executable location
if(CMAKE_GENERATOR STREQUAL "Ninja")
  file(COPY "resources" DESTINATION "${CMAKE_BINARY_DIR}")
elseif(CMAKE_GENERATOR STREQUAL "Xcode")
  file(COPY "resources" DESTINATION "${CMAKE_BINARY_DIR}/Debug")
elseif(MSVC)
  file(COPY "resources" DESTINATION "${CMAKE_BINARY_DIR}")
endif()

#
# Define Assimp library target
#
#if(APPLE AND NOT IOS)
#set(assimp_dir ${CMAKE_SOURCE_DIR}/deps/assimp/mac)
#else()
#message(FATAL_ERROR "Platform not supported yet")
#endif()

#add_library(assimp STATIC IMPORTED)
#set_target_properties(assimp
#PROPERTIES
#IMPORTED_LOCATION "${assimp_dir}/lib/libassimp.a"
#INTERFACE_INCLUDE_DIRECTORIES "${assimp_dir}/include")

add_executable(blocks
    deps/glad/src/glad.cpp
    # Core
    src/Program.hpp
    src/Program.cpp
    src/InputSystem.hpp
    src/InputSystem.cpp
    src/Camera.hpp
    src/Camera.cpp
    src/Shader.hpp
    src/PlayerInput.hpp
    src/Texture.hpp
    src/TriangleMesh.hpp
    src/OpenGL.hpp
    src/ResourceManager.hpp
    src/ResourceManager.cpp
    src/ResourceFile.hpp
    src/ResourceFile.cpp

    # Memory
    src/Memory.hpp
    src/Allocator.hpp
    src/MallocAllocator.hpp
    src/LinearAllocator.hpp

    # Utils
    src/Defines.hpp
    src/Logger.cpp
    src/Logger.hpp
    src/Utils.hpp
    src/Utils.cpp
    src/Path.hpp
    src/Path.cpp

    # Math
    src/math/Vec2.hpp
    src/math/Vec3.hpp
    src/math/Vec4.hpp
    src/math/Mat4.hpp
    src/math/Mat4.cpp
    src/math/Float.hpp
    src/math/Quaternion.hpp
    src/math/Quaternion.cpp

    # Rendering
    src/Renderer.hpp
    src/Renderer.cpp

    # FileSystem
    src/FileSystem.hpp
    src/FileSystem/Common.cpp

    # Collections
    src/Collections/Array.hpp
    src/Collections/String.hpp
    src/Collections/StringView.hpp
    src/Collections/RobinHashMap.hpp
    src/Collections/RobinHashMap.cpp

    src/main.cpp)

target_compile_definitions(blocks PRIVATE _USE_MATH_DEFINES)

if(APPLE)
    target_sources(blocks PRIVATE src/FileSystem/Apple.cpp)
elseif(WIN32)
    target_sources(blocks PRIVATE src/FileSystem/Windows.cpp)
    target_compile_definitions(blocks PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

target_compile_options(blocks PRIVATE -fno-rtti -fno-exceptions)

set_target_properties(blocks PROPERTIES CXX_STANDARD 14)

target_include_directories(blocks
  PUBLIC
    deps/glad/include
    src)

if(UNIX)
    target_link_libraries(blocks PUBLIC m)
endif()

target_link_libraries(blocks
  PUBLIC
    CONAN_PKG::sdl2)
