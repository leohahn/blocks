#include "Program.hpp"

#include "FileSystem.hpp"
#include "Logger.hpp"
#include "glad/glad.h"
#include "Sid.hpp"

void
InitProgram(Program* program, size_t memory_amount, int32_t window_width, int32_t window_height)
{
    const size_t resource_manager_designated_memory = MEGABYTES(10);
    
    program->memory = Memory(memory_amount);
    program->main_allocator = LinearAllocator("main", program->memory);
    program->temp_allocator = MallocAllocator("temporary_allocator");
    program->window_width = window_width;
    program->window_height = window_height;
    program->running = true;

    program->resource_manager_allocator = LinearAllocator(
        "resource_manager",
        program->main_allocator.Allocate(resource_manager_designated_memory),
        resource_manager_designated_memory
    );
    program->resource_manager = program->main_allocator.New<ResourceManager>(&program->resource_manager_allocator, &program->temp_allocator);
    program->resource_manager->Create();

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

    program->window =
        SDL_CreateWindow("Blocks", 0, 0, window_width, window_height, SDL_WINDOW_OPENGL);
    program->gl_context = SDL_GL_CreateContext(program->window);

    LOG_INFO("Initializing glad");
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        LOG_ERROR("Failed to initialize GLAD\n");
        exit(1);
    }

    glViewport(0, 0, window_width, window_height);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Initialize Sid database
    SidDatabase::Initialize(&program->main_allocator);
}

void
TerminateProgram(Program* program)
{
    assert(program);
    SidDatabase::Terminate();
    SDL_GL_DeleteContext(program->gl_context);
    SDL_DestroyWindow(program->window);
    program->resource_manager->Destroy();
    program->main_allocator.Delete<ResourceManager>(program->resource_manager);
    SDL_Quit();
}
