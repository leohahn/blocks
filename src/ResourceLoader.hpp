#pragma once

#include "Allocator.hpp"
#include "Collections/StringView.hpp"
#include "TriangleMesh.hpp"

namespace ResourceLoader {

TriangleMesh LoadMeshFromFile(Allocator* allocator, const StringView& path);

}
