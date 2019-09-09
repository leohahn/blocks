#include "Han/Program.hpp"

#include "Han/FileSystem.hpp"
#include "Han/Logger.hpp"
#include "Han/Sid.hpp"
#include "glad/glad.h"
#include "Han/Renderer/LowLevel.hpp"
#include "Han/Json.hpp"
#include <SDL.h>

void
InitProgram(Program* program, size_t memory_amount, int32_t window_width, int32_t window_height)
{
    const size_t resource_manager_designated_memory = MEGABYTES(64);

    program->memory = Memory(memory_amount);
    program->main_allocator = LinearAllocator("main", program->memory);
    program->temp_allocator = MallocAllocator("temporary_allocator");

    // ===============================================================
    // First, load the engine configuration file
    //
    Path config_path = FileSystem::GetResourcesPath(&program->temp_allocator);
    config_path.Push("engine_config.json");

    size_t file_size;
    uint8_t* file_data = FileSystem::LoadFileToMemory(&program->temp_allocator, config_path, &file_size);

    Json::Document config_doc(&program->temp_allocator);
    config_doc.Parse(file_data, file_size);

    if (config_doc.HasParseErrors() || !config_doc.root_val.IsObject()) {
        LOG_ERROR("Failed to parse configuration file: %s", config_doc.GetErrorStr());
        ASSERT(false, "should not enter here");
    }

    program->temp_allocator.Deallocate(file_data);

    const Json::Val* os_obj = nullptr;
#if OS_WINDOWS
    os_obj = config_doc.root_val.AsObject()->Find(String(&program->temp_allocator, "windows"));
#else
#error "not working yet"
#endif

    if (!os_obj || !os_obj->IsObject()) {
        LOG_ERROR("Os configuration was not found");
        ASSERT(false, "should not enter here");
    }

    const RobinHashMap<String, Json::Val>* config_obj = os_obj->AsObject();
    const Json::Val* game_dll_val = config_obj->Find(String(&program->temp_allocator, "game_dll"));
    if (!game_dll_val || !game_dll_val->IsString()) {
        LOG_ERROR("Failed to parse configuration file: No game dll");
        ASSERT(false, "should not enter here");
    }

    program->game_dll_path = FileSystem::GetResourcesPath(&program->main_allocator);
    program->game_dll_path.Push(game_dll_val->AsString()->View());

    // ===============================================================
    WindowOptions window_opts;
    window_opts.height = window_height;
    window_opts.width = window_width;
    window_opts.title = "Blocks";
    
    program->window = Window::Create(&program->main_allocator, window_opts);

    Graphics::LowLevelApi::Initialize(&program->main_allocator);
    Graphics::LowLevelApi::SetViewPort(0, 0, window_width, window_height);
    Graphics::LowLevelApi::SetFaceCulling(true);
    Graphics::LowLevelApi::SetDepthTest(true);

    program->running = true;

    program->resource_manager_allocator = LinearAllocator(
        "resource_manager",
        program->main_allocator.Allocate(resource_manager_designated_memory),
        resource_manager_designated_memory
    );
    program->resource_manager = program->main_allocator.New<ResourceManager>(&program->resource_manager_allocator, &program->temp_allocator);
    program->resource_manager->Create();

    SidDatabase::Initialize(&program->main_allocator);
}

void
TerminateProgram(Program* program)
{
    assert(program);
    SidDatabase::Terminate();
    program->resource_manager->Destroy();
    program->main_allocator.Delete(program->resource_manager);
    Graphics::LowLevelApi::Terminate();
    program->main_allocator.Delete(program->window);
    SDL_Quit();
}
