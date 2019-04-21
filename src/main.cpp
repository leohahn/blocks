#include "Allocator.hpp"
#include "Camera.hpp"
#include "Defines.hpp"
#include "FileSystem.hpp"
#include "InputSystem.hpp"
#include "Logger.hpp"
#include "MallocAllocator.hpp"
#include "Math.hpp"
#include "OpenGL.hpp"
#include "PlayerInput.hpp"
#include "Program.hpp"
#include "Renderer.hpp"
#include "ResourceManager.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "TriangleMesh.hpp"
#include "Utils.hpp"
#include <SDL.h>
#include <SDL_opengl.h>
#include <assert.h>
#include <glad/glad.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

static TriangleMesh
SetupPlane(Allocator* allocator, Allocator* scratch_allocator)
{
    // clang-format off
    static const uint32_t indices[] =
    {
        0, 1, 2, 2, 3, 0
    };

    static const Vec3 vertices[] =
    {
        Vec3(-1.0f, -1.0f,  0.0f), // 0
        Vec3( 1.0f, -1.0f,  0.0f), // 1
        Vec3( 1.0f,  1.0f,  0.0f), // 2
        Vec3(-1.0f,  1.0f,  0.0f), // 3
    };

    static const Vec2 uvs[] =
    {
        Vec2(0.0f, 0.0f), // 0
        Vec2(1.0f, 0.0f), // 1
        Vec2(1.0f, 1.0f), // 2
        Vec2(0.0f, 1.0f), // 3
    };

    static_assert(ARRAY_SIZE(vertices) == ARRAY_SIZE(uvs),
                  "vertices and tex_coords should be the same size");

    // clang-format on
    TriangleMesh mesh(allocator);
    mesh.name.Append("Plane");

    // Fill indices up
    for (size_t i = 0; i < ARRAY_SIZE(indices); ++i) {
        mesh.indices.PushBack(indices[i]);
    }

    // Fill vertices up
    for (size_t i = 0; i < ARRAY_SIZE(vertices); ++i) {
        mesh.vertices.PushBack(vertices[i]);
    }

    // Fill uvs up
    for (size_t i = 0; i < ARRAY_SIZE(uvs); ++i) {
        mesh.uvs.PushBack(uvs[i]);
    }

    assert(mesh.vertices.len == mesh.uvs.len);

    //--------------------------------------------
    // Mesh setup
    //--------------------------------------------

    // Build the buffer that is going to be uploaded to the GPU.
    Array<OpenGL::Vertex_PT> buffer(scratch_allocator);
    for (size_t i = 0; i < mesh.vertices.len; ++i) {
        buffer.PushBack(OpenGL::Vertex_PT(mesh.vertices[i], mesh.uvs[i]));
    }

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // create the vbo and ebo
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    // bind vbo
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(OpenGL::Vertex_PT) * buffer.len, buffer.data, GL_STATIC_DRAW);

    OpenGL::SetVertexFormat_PT();

    // bind ebo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mesh.indices.len * sizeof(uint32_t),
                 mesh.indices.data,
                 GL_STATIC_DRAW);

    buffer.Destroy();
    return mesh;
}

// TODO: receive a texture catalog
static TriangleMesh
SetupCube(Allocator* allocator, Allocator* scratch_allocator)
{
    // clang-format off
    static const uint32_t indices[] =
    {
        0,  1,  2,  2,  3,  0,
        4,  5,  6,  6,  7,  4,
        8,  9, 10, 10, 11,  8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20,
    };
    
    static const Vec3 vertices[] =
    {
        // FRONT
        Vec3(-1, -1,  1), // 0
        Vec3(1, -1,  1), // 1
        Vec3(1,  1,  1), // 2
        Vec3(-1,  1,  1), // 3
        // BACK
        Vec3(1, -1, -1), // 4
        Vec3(-1, -1, -1), // 5
        Vec3(-1,  1, -1), // 6
        Vec3(1,  1, -1), // 7
        // TOP
        Vec3(-1,  1,  1), // 8
        Vec3(1,  1,  1), // 9
        Vec3(1,  1, -1), // 10
        Vec3(-1,  1, -1), // 11
        // BOTTOM
        Vec3(1, -1,  1), // 12
        Vec3(-1, -1,  1), // 13
        Vec3(-1, -1, -1), // 14
        Vec3(1, -1, -1), // 15
        // LEFT
        Vec3(-1, -1, -1), // 16
        Vec3(-1, -1,  1), // 17
        Vec3(-1,  1,  1), // 18
        Vec3(-1,  1, -1), // 19
        // RIGHT
        Vec3(1, -1,  1), // 20
        Vec3(1, -1, -1), // 21
        Vec3(1,  1, -1), // 22
        Vec3(1,  1,  1), // 23
    };
    
    static const Vec2 uvs[] =
    {
        // FRONT
        Vec2(0, 0), // 0
        Vec2(1, 0),// 1
        Vec2(1, 1),// 2
        Vec2(0, 1),// 3
        // BACK
        Vec2(0, 0), // 4
        Vec2(1, 0), // 5
        Vec2(1, 1), // 6
        Vec2(0, 1), // 7
        // TOP
        Vec2(0, 0), // 8
        Vec2(1, 0), // 9
        Vec2(1, 1), // 10
        Vec2(0, 1), // 11
        // BOTTOM
        Vec2(0, 0), // 12
        Vec2(1, 0), // 13
        Vec2(1, 1), // 14
        Vec2(0, 1), // 15
        // LEFT
        Vec2(0, 0), // 16
        Vec2(1, 0), // 17
        Vec2(1, 1), // 18
        Vec2(0, 1), // 19
        // RIGHT
        Vec2(0, 0), // 20
        Vec2(1, 0), // 21
        Vec2(1, 1), // 22
        Vec2(0, 1), // 23
    };

    // clang-format on

    TriangleMesh mesh(allocator);
    mesh.name.Append("Cube");

    // Fill indices up
    for (size_t i = 0; i < ARRAY_SIZE(indices); ++i) {
        mesh.indices.PushBack(indices[i]);
    }

    // Fill vertices up
    for (size_t i = 0; i < ARRAY_SIZE(vertices); ++i) {
        mesh.vertices.PushBack(vertices[i]);
    }

    // Fill uvs up
    for (size_t i = 0; i < ARRAY_SIZE(uvs); ++i) {
        mesh.uvs.PushBack(uvs[i]);
    }

    assert(mesh.vertices.len == mesh.uvs.len);

    //--------------------------------------------
    // Mesh setup
    //--------------------------------------------

    // Build the buffer that is going to be uploaded to the GPU.
    Array<OpenGL::Vertex_PT> buffer(scratch_allocator);
    for (size_t i = 0; i < mesh.vertices.len; ++i) {
        buffer.PushBack(OpenGL::Vertex_PT(mesh.vertices[i], mesh.uvs[i]));
    }

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // create the vbo and ebo
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    // bind vbo
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(OpenGL::Vertex_PT) * buffer.len, buffer.data, GL_STATIC_DRAW);

    OpenGL::SetVertexFormat_PT();

    // bind ebo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mesh.indices.len * sizeof(uint32_t),
                 mesh.indices.data,
                 GL_STATIC_DRAW);

    buffer.Destroy();
    return mesh;
}

static void
OnApplicationQuit(SDL_Event ev, void* user_data)
{
    bool* running = (bool*)user_data;
    *running = false;
}

//
// What needs to be done
//  [*DONE*] - Mat4 implementation
//           - LookAt
//           - Perspective
//           - Orthogonal
//  [*DONE*] - Draw the same triangle but with correct world space
//  [*DONE*] - Make a camera that moves sideways compared to the triangle
//  [DONE] - Turn camera around
//            - Quaternions?
//            - Euler angles?
//  [DONE] - Draw a cube
//  [DONE] - Make an FPS camera
//  [DONE] - Load a texture
//  [DONE] - Load from relative paths instead of absolute ones
//  [DONE] - Keep textures accessible throughout the program's lifetime
//  [DONE] - Abstract cube into a triangle mesh
//  [DONE] - Render the cube triangle mesh
//  [DONE] - Render the floor
//  [DONE] - Rotate the floor to be in the correct position
//  [TODO] - Use a fixed top-down camera
//  [TODO] - Move the top-down camera
//

int
main()
{
    // Instantiate a new program with preallocated memory
    Program program = InitProgram(MEGABYTES(128), SCREEN_WIDTH, SCREEN_HEIGHT);

    MallocAllocator temp_allocator("temporary_allocator");
    
    const size_t resource_manager_designated_memory = MEGABYTES(10);
    LinearAllocator resource_manager_allocator(
        "resource_manager",
        program.main_allocator.Allocate(resource_manager_designated_memory),
        resource_manager_designated_memory
    );
    ResourceManager resource_manager(&resource_manager_allocator, &temp_allocator);
    resource_manager.Create();

    //
    // Load shaders
    //
    LOG_DEBUG("Loading shaders\n");

    resource_manager.LoadShader("basic.glsl");
    Shader* basic_shader = resource_manager.GetShader("basic.glsl");
    assert(basic_shader && basic_shader->IsValid() && "program should be valid");
    SetLocationsForShader(basic_shader);

    bool running = true;

    //
    // Create the input system
    //
    InputSystem input_system;
    input_system.Create(&program.main_allocator);
    input_system.AddKeyboardEventListener(
        kKeyboardEventButtonDown, SDLK_q, OnApplicationQuit, &running, &program.main_allocator);

    PlayerInput player_input;
    player_input.RegisterInputs(&input_system, &program.main_allocator);

    // DEBUG meshes for testing
    TriangleMesh floor_mesh = SetupPlane(&program.main_allocator, &temp_allocator);
    TriangleMesh cube_mesh = SetupCube(&program.main_allocator, &temp_allocator);

    Camera camera(Vec3(0, 0, 5), Vec3(0, 0, -1));

    glUseProgram(basic_shader->program);

    auto projection_matrix =
        Mat4::Perspective(60.0f, (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 500.0f);
    glUniformMatrix4fv(basic_shader->projection_location, 1, false, &projection_matrix.data[0]);

    //------------------------------
    // Create the texture catalog and textures
    //------------------------------
    // LinearAllocator texture_catalog_allocator(
    //     "texture_catalog", program.main_allocator.Allocate(MEGABYTES(10)), MEGABYTES(10));
    // TextureCatalog texture_catalog(&texture_catalog_allocator, &temp_allocator);
    resource_manager.LoadTexture("wall.jpg");

    Texture* wall_texture = resource_manager.GetTexture("wall.jpg");
    LOG_DEBUG("Loaded texture named: %s", wall_texture->name.data);
    LOG_DEBUG("       width: %d", wall_texture->width);
    LOG_DEBUG("       height: %d", wall_texture->height);

    Model cottage = resource_manager.LoadModel("cottage_obj.obj");

    LOG_DEBUG("Starting main loop");
    glClearColor(0, 0, 0, 1);

    while (running) {
        input_system.Update();

        if (input_system.ReceivedQuitEvent()) {
            LOG_INFO("Received quit event, exiting application");
            break;
        }

        if (player_input.IsMovingLeft()) {
            camera.MoveLeft(0.01f);
        }

        if (player_input.IsMovingRight()) {
            camera.MoveRight(0.01f);
        }

        if (player_input.IsTurningLeft()) {
            camera.Rotate(Vec3(0, 1, 0));
        }

        if (player_input.IsTurningRight()) {
            camera.Rotate(Vec3(0, -1, 0));
        }

        if (player_input.IsTurningAbove()) {
            camera.Rotate(camera.right);
        }

        if (player_input.IsTurningBelow()) {
            camera.Rotate(-camera.right);
        }

        if (player_input.IsMovingForwards()) {
            camera.MoveForwards(0.01f);
        }

        if (player_input.IsMovingBackwards()) {
            camera.MoveBackwards(0.01f);
        }

        //
        // Start rendering part of the main loop
        //
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // TODO: figure out a way of keeping track of the shader, especially
        // when rendering a mesh (should it be stored in the mesh?)
        glUseProgram(basic_shader->program);

        auto ticks = SDL_GetTicks();

        // TODO: is ok to calculate the view matrix every time here?
        // consider only updating the view matrix when something changed on the camera.
        OpenGL::SetUniformMatrixForCurrentShader(basic_shader->view_location,
                                                 camera.GetViewMatrix());

        // TODO: add real values here for the parameters
        Vec3 cube_position(0);
        Quaternion cube_orientation =
            Quaternion::Rotation(Math::DegreesToRadians(ticks * 0.035f), Vec3(0, 1, 0));
        float cube_scale = 1.0f;
        RenderMesh(cube_mesh, *basic_shader, cube_position, cube_orientation, cube_scale);

        Vec3 floor_position(0, -5, 3);
        Quaternion floor_orientation =
            Quaternion::Rotation(Math::DegreesToRadians(90), Vec3(-1, 0, 0));
        float floor_scale = 50.0f;
        RenderMesh(floor_mesh, *basic_shader, floor_position, floor_orientation, floor_scale);

        for (size_t i = 1; i < cottage.meshes.len; ++i) {
            RenderMesh(*cottage.meshes[i], *basic_shader, Vec3::Zero(), Quaternion::Identity(), 1.0f);
        }

        SDL_GL_SwapWindow(program.window);
    }

    LOG_INFO("Deallocating main resources");
    LOG_DEBUG("Total memory used by heap allocator: %s",
              Utils::GetPrettySize(temp_allocator.GetBytesWaterMark()));

    // TODO: remove this
    // wall_texture.name.Destroy();
    
    cottage.Destroy();

    floor_mesh.Destroy();
    cube_mesh.Destroy();
    input_system.Destroy();
    resource_manager.Destroy();
    resource_manager_allocator.Clear();
    TerminateProgram(&program);
}
