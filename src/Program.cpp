#include "Program.hpp"

#include "FileSystem.hpp"
#include "Logger.hpp"
#include "glad/glad.h"

Program
InitProgram(size_t memory_amount, int32_t window_width, int32_t window_height)
{
    Program program = {};
    program.memory = Memory(memory_amount);
    program.memory.Create();
    program.main_allocator = LinearAllocator("main", program.memory);
    program.window_width = window_width;
    program.window_height = window_height;

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
        SDL_CreateWindow("Blocks", 0, 0, window_width, window_height, SDL_WINDOW_OPENGL);
    program.gl_context = SDL_GL_CreateContext(program.window);

    LOG_INFO("Initializing glad");
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        LOG_ERROR("Failed to initialize GLAD\n");
        exit(1);
    }

    glViewport(0, 0, window_width, window_height);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    return program;
}

void
TerminateProgram(Program* program)
{
    assert(program);
    program->main_allocator.Clear();
    program->memory.Destroy();
    SDL_GL_DeleteContext(program->gl_context);
    SDL_DestroyWindow(program->window);
    SDL_Quit();
}
