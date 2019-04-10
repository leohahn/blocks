#pragma once

#include "Allocator.hpp"
#include "Collections/String.hpp"
#include "Collections/StringView.hpp"

struct Texture
{
    String name;
    uint32_t handle;

    int32_t width;
    int32_t height;
    bool loaded;

    Texture()
        : Texture(nullptr)
    {}

    Texture(Allocator* allocator)
        : name(allocator)
        , handle(0)
        , width(0)
        , height(0)
        , loaded(false)
    {}
};
