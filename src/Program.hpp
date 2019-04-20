#pragma once

#include "Path.hpp"
#include "LinearAllocator.hpp"
#include "Memory.hpp"
#include <SDL.h>

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
    bool running = true;
    SDL_Window* window = nullptr;
    int32_t window_width;
    int32_t window_height;
    SDL_GLContext gl_context;
};

Program InitProgram(size_t memory_amount, int32_t window_width, int32_t window_height);

void TerminateProgram(Program* program);
