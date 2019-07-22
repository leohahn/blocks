#include "Program.hpp"

#include "FileSystem.hpp"
#include "Logger.hpp"
#include "glad/glad.h"
#include "Sid.hpp"

void
InitProgram(Program* program, size_t memory_amount, int32_t window_width, int32_t window_height)
{
    const size_t resource_manager_designated_memory = MEGABYTES(64);
    
    program->memory = Memory(memory_amount);
    program->main_allocator = LinearAllocator("main", program->memory);
    program->temp_allocator = MallocAllocator("temporary_allocator");
    
    WindowOptions window_opts;
    window_opts.height = window_height;
    window_opts.width = window_width;
    window_opts.title = "Blocks";
    
    program->window = Window::Create(&program->main_allocator, window_opts);
    program->running = true;

    program->resource_manager_allocator = LinearAllocator(
        "resource_manager",
        program->main_allocator.Allocate(resource_manager_designated_memory),
        resource_manager_designated_memory
    );
    program->resource_manager = program->main_allocator.New<ResourceManager>(&program->resource_manager_allocator, &program->temp_allocator);
    program->resource_manager->Create();

    // Initialize Sid database
    SidDatabase::Initialize(&program->main_allocator);
}

void
TerminateProgram(Program* program)
{
    assert(program);
    SidDatabase::Terminate();
    program->resource_manager->Destroy();
    program->main_allocator.Delete<ResourceManager>(program->resource_manager);
    SDL_Quit();
}
