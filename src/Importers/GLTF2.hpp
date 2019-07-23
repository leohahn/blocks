#pragma once

#include "Allocator.hpp"
#include "ResourceManager.hpp"
#include "Path.hpp"
#include "Model.hpp"

struct ResourceManager;

Model ImportGltf2Model(Allocator* allocator, Allocator* scratch_allocator, const Path& path, ResourceManager* resource_manager, int model_index = 0);
