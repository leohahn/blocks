#pragma once

#include "Han/Allocator.hpp"
#include "Han/ResourceManager.hpp"
#include "Han/Path.hpp"
#include "Han/Model.hpp"

struct ResourceManager;

Model ImportGltf2Model(Allocator* allocator, Allocator* scratch_allocator, const Path& path, ResourceManager* resource_manager, int model_index = 0);
