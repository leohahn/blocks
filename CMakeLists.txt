cmake_minimum_required(VERSION 3.5)

project(blocks)

# Set a default build type if none was specified
set(default_build_type "Debug")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
endif()

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

add_executable(blocks
    deps/glad/src/glad.cpp
    # Core
    src/Program.hpp
    src/Program.cpp
    src/Application.hpp
    src/InputSystem.hpp
    src/InputSystem.cpp
    src/Camera.hpp
    src/Camera.cpp
    src/Shader.hpp
    src/Shader.cpp
    src/PlayerInput.hpp
    src/Texture.hpp
    src/TriangleMesh.hpp
    src/OpenGL.hpp
    src/ResourceManager.hpp
    src/ResourceManager.cpp
    src/ResourceFile.hpp
    src/ResourceFile.cpp
    src/Sid.hpp
    src/Sid.cpp
    src/Renderer/Material.hpp
    src/Renderer/Material.cpp
    src/Json.hpp
    src/Json.cpp
    src/Model.hpp
    src/Window.hpp
    src/Window.cpp
    src/Renderer/Buffer.hpp
    src/Renderer/Buffer.cpp
    src/Renderer/OpenGLBuffer.hpp
    src/Renderer/OpenGLBuffer.cpp
    src/Renderer/LowLevel.hpp
    src/Renderer/LowLevelOpenGL.hpp
    src/Renderer/LowLevelOpenGL.cpp

    # Importers
    src/Importers/GLTF2.hpp
    src/Importers/GLTF2.cpp

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
    src/Math/Vec2.hpp
    src/Math/Vec3.hpp
    src/Math/Vec4.hpp
    src/Math/Mat4.hpp
    src/Math/Mat4.cpp
    src/Math/Float.hpp
    src/Math/Quaternion.hpp
    src/Math/Quaternion.cpp

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

    src/main.cpp)

target_compile_definitions(blocks PRIVATE _USE_MATH_DEFINES)

if(APPLE)
    target_sources(blocks PRIVATE src/FileSystem/Apple.cpp)
elseif(WIN32)
	#target_sources(blocks PRIVATE src/FileSystem/Windows.cpp)
    target_compile_definitions(blocks PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if(WIN32)
    target_compile_options(blocks
      PRIVATE
        /EHsc-
        $<$<CONFIG:Debug>:/MDd>
        $<$<CONFIG:Release>:/MD>
        $<$<CONFIG:Debug>:/Zi>)
    target_compile_definitions(blocks
      PRIVATE
        _HAS_EXCEPTIONS=0
	$<$<CONFIG:Debug>:BLOCKS_DEBUG>)
else()
	#target_compile_options(blocks PRIVATE -fno-rtti -fno-exceptions)
    target_compile_options(blocks PRIVATE -fno-exceptions)
endif()

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

#
# Add game dll for testing
#
add_library(game SHARED
    example/Game.cpp)

target_include_directories(game PRIVATE src)

target_compile_definitions(game PRIVATE BLOCKS_COMPILING_DLL)

add_custom_command(
    TARGET game 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:game>
        $<TARGET_FILE_DIR:blocks>/$<TARGET_FILE_NAME:game>)
