#pragma once
#include "Allocator.hpp"

namespace FileSystem {

uint8_t* LoadFileToMemory(Allocator* allocator, const char* path, size_t* out_file_size = nullptr);

}