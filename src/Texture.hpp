#pragma once

#include "Allocator.hpp"
#include "Collections/String.hpp"

struct Texture
{
    String name;
    uint32_t handle;
    
    int32_t width;
    int32_t height;
    bool loaded;

    Texture()
        : loaded(false)
        , handle(0)
    {}
};

Texture LoadTexture(Allocator* allocator, const char* asset_path);
