#include "Han/Application.hpp"
#include "Han/Logger.hpp"
#include "Han/FileSystem.hpp"
#include "Han/Json.hpp"
#include "Han/Renderer/LowLevel.hpp"
#include "Han/ResourceManager.hpp"
#include "Han/Sid.hpp"
#include "Editor.hpp"
#include <chrono>

Application* Application::_instance = nullptr;

Application* Application::Instance()
{
	return _instance;
}

Application::Application(ApplicationParams params)
	: _params(params)
	, _window(nullptr)
	, _resource_manager(nullptr)
	, _running(false)
{
	ASSERT(params.memory_size, "Should have memory size specified");
	ASSERT(params.screen_width, "Should have screen width specified");
	ASSERT(params.screen_height, "Should have screen height specified");
	ASSERT(_instance == nullptr, "There should be only one Application");
	_instance = this;
}

void
Application::Initialize()
{
	LOG_INFO("Initializing the engine");

    const size_t resource_manager_designated_memory = MEGABYTES(64);

    _memory = Memory(_params.memory_size);
    _main_allocator = LinearAllocator("main", _memory);
    _temp_allocator = MallocAllocator("temporary_allocator");

	// TODO: consider using another allocator here.
	_layer_stack.SetAllocator(&_temp_allocator);

    // ===============================================================
    // First, load the engine configuration file
    //
    Path config_path = FileSystem::GetResourcesPath(&_temp_allocator);
    config_path.Push("engine_config.json");

    size_t file_size;
    uint8_t* file_data = FileSystem::LoadFileToMemory(&_temp_allocator, config_path, &file_size);

    Json::Document config_doc(&_temp_allocator);
    config_doc.Parse(file_data, file_size);

    if (config_doc.HasParseErrors() || !config_doc.root_val.IsObject()) {
        LOG_ERROR("Failed to parse configuration file: %s", config_doc.GetErrorStr());
        ASSERT(false, "should not enter here");
    }

    _temp_allocator.Deallocate(file_data);

    const Json::Val* os_obj = nullptr;
#if OS_WINDOWS
    os_obj = config_doc.root_val.AsObject()->Find(String(&_temp_allocator, "windows"));
#else
#error "not working yet"
#endif

    if (!os_obj || !os_obj->IsObject()) {
        LOG_ERROR("Os configuration was not found");
        ASSERT(false, "should not enter here");
    }

    const RobinHashMap<String, Json::Val>* config_obj = os_obj->AsObject();
    const Json::Val* game_dll_val = config_obj->Find(String(&_temp_allocator, "game_dll"));
    if (!game_dll_val || !game_dll_val->IsString()) {
        LOG_ERROR("Failed to parse configuration file: No game dll");
        ASSERT(false, "should not enter here");
    }

    // ===============================================================
    WindowOptions window_opts;
    window_opts.height = _params.screen_height;
    window_opts.width = _params.screen_width;
    window_opts.title = "Blocks";
	window_opts.vsync = _params.vsync;
    
    _window = Window::Create(&_main_allocator, window_opts);
	_window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

    Graphics::LowLevelApi::Initialize(&_main_allocator);
    Graphics::LowLevelApi::SetViewPort(0, 0, _params.screen_width, _params.screen_height);
    Graphics::LowLevelApi::SetFaceCulling(true);
    Graphics::LowLevelApi::SetDepthTest(true);

    _running = true;

    _resource_manager_allocator = LinearAllocator(
        "resource_manager",
        _main_allocator.Allocate(resource_manager_designated_memory),
        resource_manager_designated_memory
    );
    _resource_manager = _main_allocator.New<ResourceManager>(&_resource_manager_allocator, &_temp_allocator);
    _resource_manager->Create();

    SidDatabase::Initialize(&_main_allocator);

	_start_time = std::chrono::high_resolution_clock::now();

	Editor::Initialize();
	OnInitialize();
}

void
Application::Shutdown()
{
	OnShutdown();
	_layer_stack.Clear();

	LOG_INFO("Destroying the engine");
    SidDatabase::Terminate();
    _resource_manager->Destroy();
    _main_allocator.Delete(_resource_manager);
    Graphics::LowLevelApi::Terminate();
    _main_allocator.Delete(_window);

	Editor::Terminate();
}

void
Application::Run()
{
	Initialize();

	_time = GetTime();

	const double desired_fps = 60.0f;

    while (_running) {
		auto now = GetTime();

		DeltaTime delta;
		// TODO: find a better constant
		if (HAN_ABS(_time) < 0.0001) {
			delta = DeltaTime(1.0 / desired_fps);
		} else {
			delta = now - _time;
		}
		_time = GetTime();

		for (auto layer : _layer_stack) {
			layer->OnUpdate(delta);
		}

        _window->OnUpdate();
    }

	Shutdown();
}

void
Application::PushLayer(Layer* layer)
{
	_layer_stack.PushLayer(layer);
}

void
Application::PushOverlay(Layer* layer)
{
	_layer_stack.PushOverlay(layer);
}

void
Application::PopLayer(Layer* layer)
{
	_layer_stack.PopLayer(layer);
}

void
Application::PopOverLay(Layer* layer)
{
	_layer_stack.PopOverlay(layer);
}

void
Application::OnEvent(Event& ev)
{
	for (auto it = _layer_stack.end(); it != _layer_stack.begin();) {
		(*--it)->OnEvent(ev);
		if (ev.handled) {
			break;
		}
	}
}

Time
Application::GetTime() const
{
	using namespace std::chrono;
	auto now = high_resolution_clock::now();
	double time_sec = duration<double>(now - _start_time).count();
	return Time(time_sec);
}
