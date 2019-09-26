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
# Glad
###########################################################
add_library(Glad STATIC src/Vendor/glad/src/glad.cpp)

target_include_directories(Glad
  PUBLIC
    src/Vendor/glad/include)

###########################################################
# Dear, ImGui
###########################################################
add_library(ImGui
  STATIC
    src/Vendor/imgui/imgui.h
    src/Vendor/imgui/imgui_internal.h
    src/Vendor/imgui/imgui.cpp
    src/Vendor/imgui/imgui_demo.cpp
    src/Vendor/imgui/imgui_draw.cpp
    src/Vendor/imgui/imgui_widgets.cpp
    src/Vendor/imgui/examples/imgui_impl_sdl.h
    src/Vendor/imgui/examples/imgui_impl_sdl.cpp
    src/Vendor/imgui/examples/imgui_impl_opengl3.h
    src/Vendor/imgui/examples/imgui_impl_opengl3.cpp)

target_include_directories(ImGui
  PUBLIC
    src/Vendor/imgui)

target_link_libraries(ImGui PRIVATE Glad)

target_compile_definitions(ImGui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)

###########################################################
# Han Engine
###########################################################
add_library(Han
    # Include files
    include/Han/Han.hpp
    include/Han/Layer.hpp
    include/Han/DebugGuiLayer.hpp
    include/Han/Math/Vec2.hpp
    include/Han/Math/Vec3.hpp
    include/Han/Math/Vec4.hpp
    include/Han/Math/Mat4.hpp
    include/Han/Math/Float.hpp
    include/Han/Math/Quaternion.hpp
    include/Han/Application.hpp
    include/Han/Camera.hpp
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
    include/Han/Texture.hpp
    include/Han/InputSystem.hpp
    include/Han/Shader.hpp
    include/Han/Json.hpp
    include/Han/Path.hpp
    include/Han/Window.hpp
    include/Han/PlayerInput.hpp
    include/Han/OpenGL.hpp
    include/Han/Model.hpp
    include/Han/Renderer/Buffer.hpp
    include/Han/Renderer/LowLevel.hpp
    include/Han/Renderer.hpp
    include/Han/ResourceManager.hpp
    include/Han/Renderer/Material.hpp
    include/Han/Utils.hpp
    include/Han/TriangleMesh.hpp
    include/Han/Memory.hpp
    include/Han/ResourceFile.hpp
    include/Han/Events.hpp
    include/Han/StringBuilder.hpp
    include/Han/AllocatorFactory.hpp

    # Core
    src/Engine/Application.cpp
    src/Engine/Layer.cpp
    src/Engine/DebugGuiLayer.cpp
    src/Engine/Editor.hpp
    src/Engine/Editor.cpp
    src/Engine/EngineInterface.cpp
    src/Engine/InputSystem.cpp
    src/Engine/GameInstance.hpp
    src/Engine/Camera.cpp
    src/Engine/Shader.cpp
    src/Engine/Texture.cpp
    src/Engine/ResourceManager.cpp
    src/Engine/ResourceFile.cpp
    src/Engine/Sid.cpp
    src/Engine/Renderer/Material.cpp
    src/Engine/Json.cpp
    src/Engine/Window.cpp
    src/Engine/Renderer/Buffer.cpp
    src/Engine/Renderer/OpenGLBuffer.hpp
    src/Engine/Renderer/OpenGLBuffer.cpp
    src/Engine/Renderer/LowLevelOpenGL.hpp
    src/Engine/Renderer/LowLevelOpenGL.cpp
    src/Engine/AllocatorFactory.cpp

    # Importers
    src/Engine/Importers/GLTF2.hpp
    src/Engine/Importers/GLTF2.cpp

    # Utils
    src/Engine/Logger.cpp
    src/Engine/Utils.cpp
    src/Engine/Path.cpp

    # Math
    src/Engine/Math/Quaternion.cpp
    src/Engine/Math/Mat4.cpp

    # Rendering
    src/Engine/Renderer.cpp

    # FileSystem
    src/Engine/FileSystem/Common.cpp

    # Vendor libs
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
        _USE_MATH_DEFINES
        _CRT_SECURE_NO_WARNINGS)
elseif(APPLE)
    target_sources(Han
      PRIVATE
        src/Engine/FileSystem/Apple.cpp)
    target_link_libraries(Han PUBLIC m)
elseif(UNIX)
    target_link_libraries(Han PUBLIC m)
else()
    # target_compile_options(Han PRIVATE -fno-rtti -fno-exceptions)
    target_compile_options(Han PRIVATE -fno-exceptions)
endif()

# c++ standard
set_target_properties(Han PROPERTIES CXX_STANDARD 17)

target_compile_definitions(Han
  PRIVATE
    $<$<CONFIG:Debug>:HAN_DEBUG>)

# includes
target_include_directories(Han
  PUBLIC
    include
  PRIVATE
    src/Vendor
    src/Vendor/glad/include
    src/Engine)

target_link_libraries(Han
  PRIVATE
    CONAN_PKG::sdl2
    ImGui
    Glad)

###########################################################
# Game example
###########################################################
add_executable(Game Game/Main.cpp)

target_link_libraries(Game
  PRIVATE
    Han)

target_include_directories(Game
  PRIVATE
    Game)

target_compile_definitions(Game
  PRIVATE
    $<$<CONFIG:Debug>:HAN_DEBUG>)

if(MSVC)
    target_compile_definitions(Game PRIVATE _USE_MATH_DEFINES)
else()
    target_compile_options(Game PRIVATE -fno-exceptions)
endif()

set_target_properties(Game PROPERTIES CXX_STANDARD 17)

