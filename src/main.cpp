#include "Allocator.hpp"
#include "Defines.hpp"
#include "FileSystem.hpp"
#include "InputSystem.hpp"
#include "Logger.hpp"
#include "Math.hpp"
#include "PlayerInput.hpp"
#include "Renderer.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include <SDL.h>
#include <SDL_opengl.h>
#include <assert.h>
#include <glad/glad.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static GLuint
SetupCube(size_t* out_cube_num_indices)
{
    assert(out_cube_num_indices);

    static float vertices[] = {
        // vertices on the back of the cube
        -1,
        -1,
        -1, // 0
        1,
        -1,
        -1, // 1
        1,
        1,
        -1, // 2
        -1,
        1,
        -1, // 3
        // vertices on the front of the cube
        -1,
        -1,
        1, // 4
        1,
        -1,
        1, // 5
        1,
        1,
        1, // 6
        -1,
        1,
        1 // 7
    };

    static unsigned indices[] = {
        // front face
        4,
        5,
        6,
        6,
        7,
        4,
        // left face
        0,
        4,
        7,
        7,
        3,
        0,
        // right face
        1,
        2,
        6,
        6,
        5,
        1,
        // back face
        0,
        3,
        2,
        2,
        1,
        0,
        // bottom face
        0,
        1,
        5,
        5,
        4,
        0,
        // top face
        3,
        7,
        6,
        6,
        2,
        3,
    };

    *out_cube_num_indices = ARRAY_SIZE(indices);

    // create the vao
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // create the vbo and ebo
    GLuint vbo, ebo;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    // bind vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ARRAY_SIZE(vertices), vertices, GL_STATIC_DRAW);

    // specify how buffer data is layed out in memory
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // bind ebo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, ARRAY_SIZE(indices) * sizeof(unsigned), indices, GL_STATIC_DRAW);
    return vao;
}

static void
DrawCube(GLuint shader_program, GLuint vao, size_t num_indices)
{
    glUseProgram(shader_program);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);
}

static void
OnApplicationQuit(SDL_Event ev, void* user_data)
{
    bool* running = (bool*)user_data;
    *running = false;
}

struct Camera
{
    Camera(Vec3 position, Vec3 front)
        : _position(position)
        , _up_world(0, 1, 0)
    {
        _front.v = Math::Normalize(front);
        UpdateUpAndRightVectors();
    }

    Mat4 GetViewMatrix()
    {
        using namespace Math;
        const float epsilon = std::numeric_limits<float>::epsilon();

        Vec3 up_world;
        if (fabs(_front.v.x) < epsilon && fabs(_front.v.z) < epsilon) {
            if (_front.v.y > 0)
                up_world = Vec3(0, 0, -1);
            else
                up_world = Vec3(0, 0, 1);
        } else {
            up_world = Vec3(0, 1, 0);
        }

        _right = Normalize(Cross(_front.v, up_world));
        auto view_matrix = Mat4::LookAt(_position, _front.v, _right, _up);
        return view_matrix;
    }

    void MoveLeft(float offset) { _position -= _right * 0.01f; }

    void MoveRight(float offset) { _position += _right * 0.01f; }

    void MoveForwards(float offset) { _position += _front.v * 0.01f; }

    void MoveBackwards(float offset) { _position -= _front.v * 0.01f; }

    void Rotate(const Vec3& axis)
    {
        using namespace Math;
        float rotation_speed = 0.001f;

        _front = Quaternion::Rotate(_front, rotation_speed, Quaternion::New(0, axis));

        UpdateUpAndRightVectors();
    }

    Vec3 Position() const { return _position; }
    Vec3 Up() const { return _up; }
    Vec3 Front() const { return _front.v; }
    Vec3 Right() const { return _right; }

private:
    void UpdateUpAndRightVectors()
    {
        using namespace Math;
        const float epsilon = std::numeric_limits<float>::epsilon();

        if (fabs(_front.v.x) < epsilon && fabs(_front.v.z) < epsilon) {
            if (_front.v.y > 0) {
                _up_world = Vec3(0, 0, 1);
            } else {
                _up_world = Vec3(0, 0, -1);
            }
        } else {
            _up_world = Vec3(0, 1, 0);
        }

        _right = Normalize(Cross(_front.v, _up_world));
        _up = Normalize(Cross(_right, _front.v));
    }

private:
    Vec3 _position;
    Quaternion _front;
    Vec3 _up;
    Vec3 _right;
    Vec3 _up_world;
};

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
//  [TODO] - Load a texture
//

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

int
main()
{
    LOG_INFO("Initializing SDL");

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        LOG_ERROR("Failed to init SDL\n");
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);

    SDL_Window* window =
        SDL_CreateWindow("Blocks", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(window);

    // Now that sdl is initialized, we initialize glad to handle opengl calls.
    LOG_INFO("Initializing glad");
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        LOG_ERROR("Failed to initialize GLAD\n");
        return 1;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_CULL_FACE);

    //
    // Create total scratch memory
    //
    Memory main_memory(MEGABYTES(5));
    main_memory.Create();

    LinearAllocator main_allocator(main_memory);

    //
    // Load shaders
    //
    LOG_DEBUG("Loading shaders\n");

    LinearAllocator shaders_allocator(main_allocator.Allocate(KILOBYTES(10)), KILOBYTES(10));

    // TODO: figure out a way of abstracting the shader away
    GLuint basic_program = Shader::LoadFromFile(
        "/Users/lhahn/dev/prototypes/blocks/resources/shaders/basic.glsl", &shaders_allocator);
    GLuint view_location = glGetUniformLocation(basic_program, "view");
    GLuint model_location = glGetUniformLocation(basic_program, "model");
    GLuint projection_location = glGetUniformLocation(basic_program, "projection");

    shaders_allocator.Clear();

    assert(basic_program > 0 && "program should be valid");

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

    size_t cube_num_indices;
    GLuint cube_vao = SetupCube(&cube_num_indices);

    Camera camera(Vec3(0, 0, 5), Vec3(0, 0, -1));

    glUseProgram(basic_program);

    auto view_matrix = camera.GetViewMatrix();
    glUniformMatrix4fv(view_location, 1, false, &view_matrix.data[0]);

    auto projection_matrix =
        Mat4::Perspective(60.0f, (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 100.0f);
    glUniformMatrix4fv(projection_location, 1, false, &projection_matrix.data[0]);

    GLuint wall_texture =
        LoadTexture(&main_allocator, "/Users/lhahn/dev/prototypes/blocks/resources/wall.jpg");

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
            camera.Rotate(camera.Right());
        }

        if (player_input.IsTurningBelow()) {
            camera.Rotate(-camera.Right());
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

        glUseProgram(basic_program);

        // TODO: is ok to calculate the view matrix every time here?
        // consider only updating the view matrix when something changed on the camera.
        auto view_matrix = camera.GetViewMatrix();
        glUniformMatrix4fv(view_location, 1, GL_FALSE, &view_matrix.data[0]);

        // Here is the rendering code
        DrawCube(basic_program, cube_vao, cube_num_indices);

        SDL_GL_SwapWindow(window);
    }

    LOG_INFO("Deallocating main resources");

    main_allocator.Clear();
    input_system.Destroy();
    main_memory.Destroy();
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
