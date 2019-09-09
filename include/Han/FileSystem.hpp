#pragma once
#include "Han/Allocator.hpp"
#include "Han/Collections/String.hpp"
#include "Han/Path.hpp"
#include <stdint.h>

namespace FileSystem {

uint8_t* LoadFileToMemory(Allocator* allocator,
                          const Path& path,
                          size_t* out_file_size = nullptr);

Path GetResourcesPath(Allocator* allocator);

} // namespace FileSystem
