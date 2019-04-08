#pragma once

#include "Allocator.hpp"
#include "Collections/Array.hpp"
#include "Texture.hpp"
#include <assert.h>

struct TextureCatalog
{
    Allocator* allocator = nullptr;
    Array<Texture> textures;
    
public:
    ~TextureCatalog()
    {
        assert(!allocator);
    }
    
    void Create(Allocator* allocator)
    {
        textures.Create(allocator);
    }

    void Destroy()
    {
        assert(allocator);
        textures.Destroy();
        allocator = nullptr;
    }
};
