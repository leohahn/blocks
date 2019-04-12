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
#include "Renderer.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "TextureCatalog.hpp"
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

    TriangleListInfo list_info;
    list_info.num_indices = mesh.indices.len;
    list_info.first_index = 0;
    list_info.texture = nullptr; // TODO: pass a texture here
    mesh.triangle_list_infos.PushBack(list_info);

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

    TriangleListInfo list_info;
    list_info.num_indices = mesh.indices.len;
    list_info.first_index = 0;
    list_info.texture = nullptr; // TODO: pass a texture here
    mesh.triangle_list_infos.PushBack(list_info);

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
//  [TODO] - Render the floor
//  [TODO] - Use a fixed top-down camera
//  [TODO] - Move the top-down camera
//

enum class ProgramState
{
    Game, Menu, Editor,
};

struct Program
{
    ProgramState state = ProgramState::Game;
    Memory memory;
    bool running = true;
    SDL_Window* window = nullptr;
    SDL_GLContext gl_context;
};

static Program
InitProgram(size_t memory_amount)
{
    Program program;
    program.memory = Memory(memory_amount);
    program.memory.Create();

    LOG_INFO("Initializing SDL");

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        LOG_ERROR("Failed to init SDL\n");
        exit(1);
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        LOG_ERROR("Failed to init SDL\n");
        exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);
    
    program.window =
        SDL_CreateWindow("Blocks", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    program.gl_context = SDL_GL_CreateContext(program.window);

    LOG_INFO("Initializing glad");
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        LOG_ERROR("Failed to initialize GLAD\n");
        exit(1);
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);  
    
    return program;
}

static void
TerminateProgram(Program* program)
{
    assert(program);
    program->memory.Destroy();
    SDL_GL_DeleteContext(program->gl_context);
    SDL_DestroyWindow(program->window);
    SDL_Quit();
}

int
main()
{
    // Instantiate a new program
    Program program = InitProgram(MEGABYTES(128));

    //
    // Create total scratch memory
    //
    LinearAllocator main_allocator("main", program.memory);

    //
    // Load shaders
    //
    LOG_DEBUG("Loading shaders\n");

    LinearAllocator shaders_allocator(
        "shaders", main_allocator.Allocate(KILOBYTES(10)), KILOBYTES(10));

    Shader basic_shader = Shader::LoadFromFile(
        "/Users/lhahn/dev/prototypes/blocks/resources/shaders/basic.glsl", &shaders_allocator);
    assert(basic_shader.IsValid() > 0 && "program should be valid");
    SetLocationsForShader(&basic_shader);
    shaders_allocator.Clear();

    // TODO, FIXME: instead of using malloc here, use a better defined allocator, like a frame one.
    MallocAllocator temp_allocator("temporary_allocator");

    bool running = true;

    //
    // Create the input system
    //
    InputSystem input_system;
    input_system.Create(&main_allocator);
    input_system.AddKeyboardEventListener(
        kKeyboardEventButtonDown, SDLK_q, OnApplicationQuit, &running, &main_allocator);

    PlayerInput player_input;
    player_input.RegisterInputs(&input_system, &main_allocator);

    TriangleMesh floor_mesh = SetupPlane(&main_allocator, &temp_allocator);
    TriangleMesh cube_mesh = SetupCube(&main_allocator, &temp_allocator);

    Camera camera(Vec3(0, 0, 5), Vec3(0, 0, -1));

    glUseProgram(basic_shader.program);

    auto projection_matrix =
        Mat4::Perspective(60.0f, (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 100.0f);
    glUniformMatrix4fv(basic_shader.projection_location, 1, false, &projection_matrix.data[0]);

    LinearAllocator texture_catalog_allocator(
        "texture_catalog", main_allocator.Allocate(MEGABYTES(10)), MEGABYTES(10));
    TextureCatalog texture_catalog(&texture_catalog_allocator, &temp_allocator);
    texture_catalog.LoadTexture("wall.jpg");

    Texture* wall_texture = texture_catalog.GetTexture("wall.jpg");
    LOG_DEBUG("Loaded texture named: %s", wall_texture->name.data);
    LOG_DEBUG("       width: %d", wall_texture->width);
    LOG_DEBUG("       height: %d", wall_texture->height);

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
            // camera.Rotate(camera.Up());
            camera.Rotate(Vec3(0, 1, 0));
        }

        if (player_input.IsTurningRight()) {
            // camera.Rotate(-camera.Up());
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
        glUseProgram(basic_shader.program);

        auto ticks = SDL_GetTicks();

        // TODO: is ok to calculate the view matrix every time here?
        // consider only updating the view matrix when something changed on the camera.
        OpenGL::SetUniformMatrixForCurrentShader(basic_shader.view_location,
                                                 camera.GetViewMatrix());

        // TODO: add real values here for the parameters
        Vec3 cube_position(0);
        cube_position.x += ticks * 0.001f;

        Quaternion cube_orientation;
        float cube_scale = 1.0f;

        RenderMesh(cube_mesh, basic_shader, cube_position, cube_orientation, cube_scale);
        
        Vec3 floor_position(0);
        floor_position.z -= ticks * 0.001f;

        Quaternion floor_orientation;
        float floor_scale = 10.0f;

        RenderMesh(floor_mesh, basic_shader, floor_position, floor_orientation, floor_scale);

        SDL_GL_SwapWindow(program.window);
    }

    LOG_INFO("Deallocating main resources");
    LOG_DEBUG("Total memory used by heap allocator: %s",
              Utils::GetPrettySize(temp_allocator.GetBytesWaterMark()));

    // TODO: remove this
    // wall_texture.name.Destroy();

    texture_catalog.Destroy();
    texture_catalog_allocator.Clear();
    main_allocator.Clear();
    input_system.Destroy();
    TerminateProgram(&program);
}
