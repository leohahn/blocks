#pragma once

#include "Allocator.hpp"
#include "ResourceManager.hpp"
#include "Path.hpp"
#include "Model.hpp"

Model ImportGltf2Model(Allocator* allocator, Allocator* scratch_allocator, const Path& path);
