#include <stdio.h>
#include <assert.h>

#include <SDL.h>
#include <glad/glad.h>
#include <SDL_opengl.h>

#include "defines.hpp"
#include "logger.hpp"
#include "input_system.hpp"
#include "allocator.hpp"
#include "math/vec3.hpp"

static GLuint
LoadShaderFromFile(const char* path, LinearAllocator allocator)
{
    assert(path);

    LOG_DEBUG("Making shader program for %s\n", path);

    GLuint vertex_shader, fragment_shader, program = 0;
    GLchar info[512] = {};
    GLint success;

    char* shader_string = nullptr;
    {
        FILE* fp = fopen(path, "rb");
        assert(fp && "shader file should exist");

        // take the size of the file
        fseek(fp, 0, SEEK_END);
        size_t file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        // allocate enough memory
        shader_string = (char*)allocator.Allocate(file_size);
        assert(shader_string && "there should be enough memory here");

        // read the file into the buffer
        size_t nread = fread(shader_string, 1, file_size, fp);
        assert(nread == file_size && "the correct amount of bytes should be read");

        fclose(fp);
    }

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    if (vertex_shader == 0 || fragment_shader == 0) {
        LOG_ERROR("failed to create shaders (glCreateShader)\n");
        goto cleanup_shaders;
    }

    {
        const char *vertex_string[3] = {
            "#version 330 core\n",
            "#define VERTEX_SHADER\n",
            shader_string,
        };
        glShaderSource(vertex_shader, 3, &vertex_string[0], NULL);

        const char *fragment_string[3] = {
            "#version 330 core\n",
            "#define FRAGMENT_SHADER\n",
            shader_string,
        };
        glShaderSource(fragment_shader, 3, &fragment_string[0], NULL);
    }

    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, NULL, info);
        LOG_ERROR("Vertex shader compilation failed: %s\n", info);
        program = 0;
        goto cleanup_shaders;
    }

    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, NULL, info);
        LOG_ERROR("Fragment shader compilation failed: %s\n", info);
        program = 0;
        goto cleanup_shaders;
    }

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, nullptr, info);
        LOG_ERROR("Shader linking failed: %s\n", info);
        program = 0;
        goto cleanup_shaders;
    }

cleanup_shaders:
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

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
OnAButtonHold(SDL_Event ev, void* user_data)
{
    LOG_INFO("A button is being hold");
}

static void
OnAButtonPress(SDL_Event ev, void* user_data)
{
    LOG_INFO("A button was pressed");
}

static void
OnAButtonRelease(SDL_Event ev, void* user_data)
{
    LOG_INFO("A button was released");
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

    static Camera New()
    {
        Camera c = {};
        return c;
    }
};

//
// What needs to be done
//  [DONE] - Draw a triangle
//  [DONE] - Prototype the input system
//  [DONE] - Basic math vectors
//  [TODO] - Mat4 implementation
//  [TODO] - Draw the same triangle but with correct world space
//  [TODO] - Make a camera that moves sideways compared to the triangle
//  [TODO] - Make an FPS camera
//  [TODO] - Draw a cube
//

int
main()
{
    LOG_INFO("Initializing SDL");

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        LOG_ERROR("Failed to init SDL\n");
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("my window", 0, 0, 800, 600, SDL_WINDOW_OPENGL);

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

    glViewport(0, 0, 800, 600);

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

    GLuint program = LoadShaderFromFile(
        "/Users/lhahn/dev/prototypes/blocks/resources/shaders/basic.glsl", shaders_allocator
    );

    shaders_allocator.Clear();

    assert(program > 0 && "program should be valid");

    LOG_DEBUG("Starting main loop");

    glClearColor(0, 0, 0, 1);

    bool running = true;

    //
    // Create the input system
    //
    auto input_system = InputSystem::Make(main_allocator);
    input_system->AddKeyboardEventListener(kKeyboardEventButtonHold, SDLK_a, OnAButtonHold, nullptr, main_allocator);
    input_system->AddKeyboardEventListener(kKeyboardEventButtonDown, SDLK_a, OnAButtonPress, nullptr, main_allocator);
    input_system->AddKeyboardEventListener(kKeyboardEventButtonUp, SDLK_a, OnAButtonRelease, nullptr, main_allocator);
    input_system->AddKeyboardEventListener(kKeyboardEventButtonDown, SDLK_q, OnApplicationQuit, nullptr, main_allocator);

    GLuint triangle_vao = SetupTriangle();

    while (running) {
        input_system->Update();

        if (input_system->ReceivedQuitEvent()) {
            LOG_INFO("Received quit signal, exiting application");
            break;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Here is the rendering code
        DrawTriangle(program, triangle_vao);

        SDL_GL_SwapWindow(window);
    }

    Memory::Delete(main_memory);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
