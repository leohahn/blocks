#include <stdio.h>
#include <assert.h>

#include <SDL.h>
#include <glad/glad.h>
#include <SDL_opengl.h>

#include "defines.hpp"
#include "logger.hpp"
#include "input_system.hpp"
#include "allocator.hpp"
#include "math.hpp"
#include "shader.hpp"

static GLuint
SetupCube(size_t* cube_num_indices)
{
    assert(cube_num_indices);

    static float vertices[] = {
        // vertices on the back of the cube
        -1, -1, -1,  // 0
         1, -1, -1,  // 1
         1,  1, -1,  // 2
        -1,  1, -1,  // 3
        // vertices on the front of the cube
        -1, -1,  1,  // 4
         1, -1,  1,  // 5
         1,  1,  1,  // 6
        -1,  1,  1   // 7
    };

    static unsigned indices[] = {
        // front face
        4, 5, 6,
        6, 7, 4,
        // left face
        0, 4, 7,
        7, 3, 0
        // TODO: finish the other faces
    };

    *cube_num_indices = ARRAY_SIZE(indices);

    // create the vao
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glad_glBindVertexArray(vao);

    // create the vbo and ebo
    GLuint vbo, ebo;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    // bind vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*ARRAY_SIZE(vertices), vertices, GL_STATIC_DRAW);

    // specify how buffer data is layed out in memory
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // bind ebo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ARRAY_SIZE(indices)*sizeof(unsigned), indices, GL_STATIC_DRAW);
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
    static Camera New(Vec3 position, Vec3 front)
    {
        Camera c = {};
        c._position = position;
        c._front.v = Math::Normalize(front);
        c.UpdateUpAndRightVectors();
        return c;
    }

    Mat4 GetViewMatrix() const
    {
        auto view_matrix = Mat4::LookAt(_position, _position + _front.v, _up);
        return view_matrix;
    }

    void MoveLeft(float offset)
    {
        _position -= _right * 0.01f;
    }

    void MoveRight(float offset)
    {
        _position += _right * 0.01f;
    }

    void MoveForwards(float offset)
    {
        _position += _front.v * 0.01f;
    }

    void MoveBackwards(float offset)
    {
        _position -= _front.v * 0.01f;
    }

    void Rotate(const Vec3& axis)
    {
        float rotation_speed = 0.001f;
        auto rotated_front = Quaternion::Rotate(
            _front, rotation_speed, Quaternion::New(0, axis)
        );
        _front = rotated_front;
        UpdateUpAndRightVectors();
    }

    Vec3 Position() const { return _position; }
    Vec3 Up() const { return _up; }
    Vec3 Front() const { return _front.v; }

private:
    void UpdateUpAndRightVectors()
    {
        auto up_world = Vec3::New(0, 1, 0);
        _right = Math::Normalize(Math::Cross(_front.v, up_world));
        _up = Math::Normalize(Math::Cross(_right, _front.v));
    }

private:
    Vec3 _position;
    Quaternion _front;
    Vec3 _up;
    Vec3 _right;
};

//
// What needs to be done
//  [*DONE*] - Mat4 implementation
//           - LookAt
//           - Perspective
//           - Orthogonal
//  [*DONE*] - Draw the same triangle but with correct world space
//  [*DONE*] - Make a camera that moves sideways compared to the triangle
//  [TODO] - Turn camera around
//            - Quaternions?
//            - Euler angles?
//  [TODO] - Draw a cube
//  [TODO] - Quaternions?  
//  [TODO] - Make an FPS camera
//

class PlayerInput
{
public:
    static PlayerInput New()
    {
        PlayerInput pi = {};
        return pi;
    }

    void RegisterInputs(InputSystem* input_system, LinearAllocator& allocator)
    {
        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonHold, SDLK_a, LeftInput, this, allocator);
        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonUp, SDLK_a, LeftRelease, this, allocator);

        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonHold, SDLK_d, RightInput, this, allocator);
        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonUp, SDLK_d, RightRelease, this, allocator);

        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonHold, SDLK_RIGHT, TurnRightInput, this, allocator);
        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonUp, SDLK_RIGHT, TurnRightRelease, this, allocator);

        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonHold, SDLK_LEFT, TurnLeftInput, this, allocator);
        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonUp, SDLK_LEFT, TurnLeftRelease, this, allocator);

        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonHold, SDLK_s, BackInput, this, allocator);
        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonUp, SDLK_s, BackRelease, this, allocator);

        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonHold, SDLK_w, ForwardInput, this, allocator);
        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonUp, SDLK_w, ForwardRelease, this, allocator);
    }

    bool IsMovingLeft() const { return _moving_left; }
    bool IsMovingRight() const { return _moving_right; }
    bool IsMovingForwards() const { return _moving_forwards; }
    bool IsMovingBackwards() const { return _moving_backwards; }
    bool IsTurningLeft() const { return _turning_left; }
    bool IsTurningRight() const { return _turning_right; }

private:
    static void LeftInput(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_moving_left = true;
    }

    static void LeftRelease(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_moving_left = false;
    }

    static void RightInput(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_moving_right = true;
    }

    static void RightRelease(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_moving_right = false;
    }

    static void TurnRightInput(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_turning_right = true;
    }

    static void TurnRightRelease(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_turning_right = false;
    }

    static void TurnLeftInput(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_turning_left = true;
    }

    static void TurnLeftRelease(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_turning_left = false;
    }

    static void BackInput(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_moving_backwards = true;
    }

    static void BackRelease(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_moving_backwards = false;
    }

    static void ForwardInput(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_moving_forwards = true;
    }

    static void ForwardRelease(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_moving_forwards = false;
    }

    bool _moving_left;
    bool _moving_right;
    bool _moving_backwards;
    bool _moving_forwards;
    bool _turning_right;
    bool _turning_left;
};

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

    SDL_Window* window = SDL_CreateWindow("Blocks", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(window);

    // Now that sdl is initialized, we initialize glad to handle opengl calls.
    LOG_INFO("Initializing glad");
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        LOG_ERROR("Failed to initialize GLAD\n");
        return 1;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    //
    // Create total scratch memory
    //
    Memory main_memory = Memory::New(MEGABYTES(5));
    auto main_allocator = LinearAllocator::Make(main_memory);

    //
    // Load shaders
    //
    LOG_DEBUG("Loading shaders\n");

    auto shaders_allocator = LinearAllocator::Make(
        main_allocator.Allocate(KILOBYTES(10)), KILOBYTES(10)
    );

    // TODO: figure out a way of abstracting the shader away
    GLuint basic_program = Shader::LoadFromFile(
        "/Users/lhahn/dev/prototypes/blocks/resources/shaders/basic.glsl", shaders_allocator
    );
    auto view_location = glGetUniformLocation(basic_program, "view");
    auto model_location = glGetUniformLocation(basic_program, "model");
    auto projection_location = glGetUniformLocation(basic_program, "projection");

    shaders_allocator.Clear();

    assert(basic_program > 0 && "program should be valid");

    bool running = true;
    //
    // Create the input system
    //
    auto input_system = InputSystem::Make(main_allocator);
    input_system->AddKeyboardEventListener(kKeyboardEventButtonDown, SDLK_q, OnApplicationQuit, &running, main_allocator);

    auto player_input = PlayerInput::New();
    player_input.RegisterInputs(input_system, main_allocator);

    size_t cube_num_indices;
    GLuint cube_vao = SetupCube(&cube_num_indices);

    auto camera = Camera::New(Vec3::New(0, 0, 5), Vec3::New(0, 0, -1));

    glUseProgram(basic_program);

    auto view_matrix = camera.GetViewMatrix();
    glUniformMatrix4fv(view_location, 1, false, &view_matrix.data[0]);

    auto projection_matrix = Mat4::Perspective(60.0f, (float)SCREEN_WIDTH/SCREEN_HEIGHT, 0.1f, 100.0f);
    glUniformMatrix4fv(projection_location, 1, false, &projection_matrix.data[0]);

    LOG_DEBUG("Starting main loop");
    glClearColor(0, 0, 0, 1);
    while (running) {
        input_system->Update();

        if (input_system->ReceivedQuitEvent()) {
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
            camera.Rotate(camera.Up());
        }

        if (player_input.IsTurningRight()) {
            camera.Rotate(-camera.Up());
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

    Memory::Delete(main_memory);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
