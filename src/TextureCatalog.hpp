#pragma once

#include "Allocator.hpp"
#include "Collections/Array.hpp"
#include "Collections/StringView.hpp"
#include "Texture.hpp"
#include <assert.h>

struct TextureCatalog
{
    Allocator* allocator = nullptr;
    Allocator* scratch_allocator = nullptr;
    Array<Texture*> textures;

public:
    TextureCatalog(Allocator* allocator, Allocator* scratch_allocator)
        : allocator(allocator)
        , scratch_allocator(scratch_allocator)
        , textures(allocator)
    {}

    ~TextureCatalog() { assert(!allocator); }

    void Destroy();

    void LoadTexture(const StringView& texture_file);
    Texture* GetTexture(const StringView& name);
};
