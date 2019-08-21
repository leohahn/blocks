#pragma once

#include "Allocator.hpp"
#include "Application.hpp"

class GameInstance
{
public:
    virtual ~GameInstance() = default;
    virtual bool Load() = 0;
    virtual Application* GetApplication() const = 0;

    static GameInstance* Create(Allocator* allocator, const char* lib_name);
};
