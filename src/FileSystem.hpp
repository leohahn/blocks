#pragma once
#include "Allocator.hpp"
#include "Collections/String.hpp"
#include "Path.hpp"
#include <stdint.h>

namespace FileSystem {

uint8_t* LoadFileToMemory(Allocator* allocator,
                          const Path& path,
                          size_t* out_file_size = nullptr);

Path GetResourcesPath(Allocator* allocator);

} // namespace FileSystem
