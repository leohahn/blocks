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
SetupTriangle()
{
    static float vertices[] = {
        -0.5, -0.5, 0,
        0.5, -0.5, 0,
        0, 0.5, 0,
    };

    static unsigned indices[] = {
        0, 1, 2,
    };

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*sizeof(vertices), vertices, GL_STATIC_DRAW);

    // specify how buffer data is layed out in memory
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // bind ebo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned)*sizeof(indices), indices, GL_STATIC_DRAW);
    return vao;
}

static void
DrawTriangle(GLuint shader_program, GLuint vao)
{
    glUseProgram(shader_program);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
}

static void
OnApplicationQuit(SDL_Event ev, void* user_data)
{
    bool* running = (bool*)user_data;
    *running = false;
}

struct Camera
{
    Vec3 position;
    Vec3 front;

    static Camera New(Vec3 position, Vec3 front)
    {
        Camera c = {};
        c.position = position;
        c.front = Math::Normalize(front - position);
        return c;
    }

    Mat4 GetViewMatrix() const
    {
        auto view_matrix = Mat4::LookAt(position, position + front, Vec3::New(0, 1, 0));
        return view_matrix;
    }

    void MoveLeft(float offset)
    {
        position.x -= offset;
    }

    void MoveRight(float offset)
    {
        position.x += offset;
    }
};

//
// What needs to be done
//  [DONE] - Draw a triangle
//  [DONE] - Prototype the input system
//  [DONE] - Basic math vectors
//  [DONE] - Mat4 implementation
//           - LookAt
//           - Perspective
//           - Orthogonal
//  [DONE] - Draw the same triangle but with correct world space
//  [DONE] - Make a camera that moves sideways compared to the triangle
//  [TODO] - Quaternions?  
//  [TODO] - Make an FPS camera
//  [TODO] - Draw a cube
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
    }

    bool IsMovingLeft() const { return _moving_left; }
    bool IsMovingRight() const { return _moving_right; }

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

    bool _moving_left;
    bool _moving_right;
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

    SDL_Window* window = SDL_CreateWindow("my window", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);

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

    GLuint basic_program = Shader::LoadFromFile(
        "/Users/lhahn/dev/prototypes/blocks/resources/shaders/basic.glsl", shaders_allocator
    );
    auto view_location = glGetUniformLocation(basic_program, "view");
    auto model_location = glGetUniformLocation(basic_program, "model");
    auto projection_location = glGetUniformLocation(basic_program, "projection");

    shaders_allocator.Clear();

    assert(basic_program > 0 && "program should be valid");

    LOG_DEBUG("Starting main loop");

    glClearColor(0, 0, 0, 1);

    bool running = true;

    //
    // Create the input system
    //
    auto input_system = InputSystem::Make(main_allocator);
    input_system->AddKeyboardEventListener(kKeyboardEventButtonDown, SDLK_q, OnApplicationQuit, &running, main_allocator);

    auto player_input = PlayerInput::New();
    player_input.RegisterInputs(input_system, main_allocator);

    GLuint triangle_vao = SetupTriangle();

    auto camera = Camera::New(Vec3::New(0, 0, 5), Vec3::New(0, 0, -1));

    glUseProgram(basic_program);

    auto view_matrix = camera.GetViewMatrix();
    glUniformMatrix4fv(view_location, 1, false, &view_matrix.data[0]);

    auto projection_matrix = Mat4::Perspective(80.0f, (float)SCREEN_WIDTH/SCREEN_HEIGHT, 0.1f, 100.0f);
    glUniformMatrix4fv(projection_location, 1, false, &projection_matrix.data[0]);

    while (running) {
        input_system->Update();

        if (input_system->ReceivedQuitEvent()) {
            LOG_INFO("Received quit signal, exiting application");
            break;
        }

        if (player_input.IsMovingLeft()) {
            camera.MoveLeft(0.01f);
        }

        if (player_input.IsMovingRight()) {
            camera.MoveRight(0.01f);
        }

        //
        // Start rendering part of the main loop
        //

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(basic_program);

        auto view_matrix = camera.GetViewMatrix();
        glUniformMatrix4fv(view_location, 1, false, &view_matrix.data[0]);

        // Here is the rendering code
        DrawTriangle(basic_program, triangle_vao);

        SDL_GL_SwapWindow(window);
    }

    LOG_INFO("Deallocating main resources");

    Memory::Delete(main_memory);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
