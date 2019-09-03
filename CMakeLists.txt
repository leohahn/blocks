cmake_minimum_required(VERSION 3.5)

project(Han)

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

###########################################################
# Han Engine
###########################################################
add_library(HanInterface INTERFACE)
target_include_directories(HanInterface INTERFACE include)

add_library(Han
    # Include files
    include/Han/Han.hpp
    include/Han/Math/Vec2.hpp
    include/Han/Math/Vec3.hpp
    include/Han/Math/Vec4.hpp
    include/Han/Math/Mat4.hpp
    include/Han/Math/Float.hpp
    include/Han/Math/Quaternion.hpp
    include/Han/Application.hpp
    include/Han/Sid.hpp
    include/Han/Allocator.hpp
    include/Han/MallocAllocator.hpp
    include/Han/LinearAllocator.hpp
    include/Han/Core.hpp
    include/Han/Logger.hpp
    include/Han/FileSystem.hpp
    include/Han/Collections/Array.hpp
    include/Han/Collections/String.hpp
    include/Han/Collections/StringView.hpp
    include/Han/Collections/RobinHashMap.hpp
    include/Han/EngineInterface.hpp

    # Core
    src/Engine/Program.hpp
    src/Engine/Program.cpp
    src/Engine/EngineInterface.cpp
    src/Engine/InputSystem.hpp
    src/Engine/InputSystem.cpp
    src/Engine/GameInstance.hpp
    src/Engine/Camera.hpp
    src/Engine/Camera.cpp
    src/Engine/Shader.hpp
    src/Engine/Shader.cpp
    src/Engine/PlayerInput.hpp
    src/Engine/Texture.hpp
    src/Engine/Texture.cpp
    src/Engine/TriangleMesh.hpp
    src/Engine/OpenGL.hpp
    src/Engine/ResourceManager.hpp
    src/Engine/ResourceManager.cpp
    src/Engine/ResourceFile.hpp
    src/Engine/ResourceFile.cpp
    src/Engine/Sid.cpp
    src/Engine/Renderer/Material.hpp
    src/Engine/Renderer/Material.cpp
    src/Engine/Json.hpp
    src/Engine/Json.cpp
    src/Engine/Model.hpp
    src/Engine/Window.hpp
    src/Engine/Window.cpp
    src/Engine/Renderer/Buffer.hpp
    src/Engine/Renderer/Buffer.cpp
    src/Engine/Renderer/OpenGLBuffer.hpp
    src/Engine/Renderer/OpenGLBuffer.cpp
    src/Engine/Renderer/LowLevel.hpp
    src/Engine/Renderer/LowLevelOpenGL.hpp
    src/Engine/Renderer/LowLevelOpenGL.cpp

    # Importers
    src/Engine/Importers/GLTF2.hpp
    src/Engine/Importers/GLTF2.cpp

    # Memory
    src/Engine/Memory.hpp

    # Utils
    src/Engine/Logger.cpp
    src/Engine/Utils.hpp
    src/Engine/Utils.cpp
    src/Engine/Path.hpp
    src/Engine/Path.cpp

    # Math
    src/Engine/Math/Quaternion.cpp
    src/Engine/Math/Mat4.cpp

    # Rendering
    src/Engine/Renderer.hpp
    src/Engine/Renderer.cpp

    # FileSystem
    src/Engine/FileSystem/Common.cpp

    # Vendor libs
    src/Vendor/glad/src/glad.cpp
    src/Vendor/stb_image.cpp)

# conditional properties
if(WIN32)
    target_sources(Han PRIVATE src/Engine/WindowsGameInstance.cpp)
    target_compile_options(Han
      PRIVATE
        /EHsc-
        $<$<CONFIG:Debug>:/MDd>
        $<$<CONFIG:Release>:/MD>
        $<$<CONFIG:Debug>:/Zi>)
    target_compile_definitions(Han
      PRIVATE
        _HAS_EXCEPTIONS=0
	      $<$<CONFIG:Debug>:HAN_DEBUG>
        _USE_MATH_DEFINES
        _CRT_SECURE_NO_WARNINGS)
elseif(UNIX)
    target_link_libraries(Han PUBLIC m)
elseif(APPLE)
    target_sources(Han PRIVATE src/Engine/FileSystem/Apple.cpp)
else()
    # target_compile_options(Han PRIVATE -fno-rtti -fno-exceptions)
    target_compile_options(Han PRIVATE -fno-exceptions)
endif()

# c++ standard
set_target_properties(Han PROPERTIES CXX_STANDARD 14)

# includes
target_include_directories(Han
  PUBLIC
    include
  PRIVATE
    src/Vendor
    src/Vendor/glad/include
    src/Engine)

target_link_libraries(Han PUBLIC HanInterface PRIVATE CONAN_PKG::sdl2)

###########################################################
# Han Editor
###########################################################
add_executable(HanEditor src/Editor/Main.cpp)

target_link_libraries(HanEditor PRIVATE Han)

target_include_directories(HanEditor
  PRIVATE
    # The editor cheats and has access to all include files of the engine
    src/Engine
    src/Editor)

target_compile_definitions(HanEditor PRIVATE _USE_MATH_DEFINES)

set_target_properties(HanEditor PROPERTIES CXX_STANDARD 14)

###########################################################
# Game
###########################################################
add_library(Game SHARED
    example/Game.cpp)

target_include_directories(Game PRIVATE src)

target_compile_definitions(Game PRIVATE HAN_COMPILING_DLL)

target_link_libraries(Game PRIVATE HanInterface)

# add_custom_command(
#     TARGET game 
#     POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy
#         $<TARGET_FILE:game>
#         $<TARGET_FILE_DIR:blocks>/$<TARGET_FILE_NAME:game>)
