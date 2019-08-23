#pragma once

#include "Path.hpp"
#include "Han/LinearAllocator.hpp"
#include "Han/MallocAllocator.hpp"
#include "Memory.hpp"
#include "ResourceManager.hpp"
#include "Window.hpp"

enum class ProgramState
{
    Game = 0,
    Menu,
    Editor,
};

struct Program
{
    ProgramState state;
    Memory memory;
    LinearAllocator main_allocator;
    MallocAllocator temp_allocator;
    bool running = true;
    Window* window;
    LinearAllocator resource_manager_allocator;
    ResourceManager* resource_manager;
    Path game_dll_path;
};

void InitProgram(Program* program, size_t memory_amount, int32_t window_width, int32_t window_height);

void TerminateProgram(Program* program);
