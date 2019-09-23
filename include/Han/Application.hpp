#pragma once

#include "Han/Allocator.hpp"
#include "Han/LinearAllocator.hpp"
#include "Han/MallocAllocator.hpp"
#include "Han/Window.hpp"
#include "Han/Memory.hpp"
#include "Han/Events.hpp"
#include "Han/Layer.hpp"
#include "Han/ResourceManager.hpp"
#include <chrono>

struct ApplicationParams
{
	size_t memory_size = 0;
	uint32_t screen_width = 0;
	uint32_t screen_height = 0;
	bool vsync = true;
};

class Application
{
public:
	Application(ApplicationParams params);

	virtual ~Application() = default;
	virtual void OnInitialize() = 0;
    virtual void OnShutdown() = 0;

	Time GetTime() const;

	void Run();

	Allocator* GetLayerAllocator() { return _layer_stack.GetAllocator(); }
	void PushLayer(Layer* layer);
	void PopLayer(Layer* layer);
	void PushOverlay(Layer* layer);
	void PopOverLay(Layer* layer);

	void Quit() { _running = false; }

	static Application* Instance();

	uint32_t GetScreenWidth() const { return _params.screen_width; }
	uint32_t GetScreenHeight() const { return _params.screen_height; }
	float GetScreenAspectRatio() const { return (float)_params.screen_width / _params.screen_height; }
	ResourceManager* GetResourceManager() const { return _resource_manager; }
	Allocator* GetMainAllocator() { return &_main_allocator; }
	Allocator* GetTempAllocator() { return &_temp_allocator; }

private:
	void Initialize();
	void Shutdown();
	void OnEvent(Event& ev);

	static Application* _instance;

private:
	ApplicationParams _params;
	Memory _memory;
	Window* _window;
    LinearAllocator _resource_manager_allocator;
	ResourceManager* _resource_manager;
	bool _running;
	std::chrono::high_resolution_clock::time_point _start_time;
	Time _time;

protected:
    LinearAllocator _main_allocator;
    MallocAllocator _temp_allocator;
	LayerStack _layer_stack;
};
