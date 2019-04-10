#pragma once
#include "Allocator.hpp"
#include "Collections/String.hpp"
#include "Collections/StringView.hpp"

namespace FileSystem {

uint8_t* LoadFileToMemory(Allocator* allocator,
                          const StringView& path,
                          size_t* out_file_size = nullptr);

String GetResourcesPath(Allocator* allocator);

String JoinPaths(Allocator* allocator, const StringView& p1, const StringView& p2);

} // namespace FileSystem
