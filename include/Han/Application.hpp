#pragma once

#include "Han/Allocator.hpp"
#include "Han/LinearAllocator.hpp"
#include "Han/MallocAllocator.hpp"
#include "Han/Window.hpp"
#include "Han/Memory.hpp"
#include <chrono>

struct DeltaTime
{
public:
	DeltaTime() : DeltaTime(0.0f) {}
	DeltaTime(float t) : _delta_sec(t) {}

	operator double() const { return _delta_sec; }
	double InMilliseconds() const { return _delta_sec * 1000.0; }

private:
	double _delta_sec;
};

struct Time
{
public:
	Time() : Time(0.0) {}
	Time(double sec)
		: _time_seconds(sec)
	{}

	operator double() const { return _time_seconds; }

	DeltaTime operator-(Time other)
	{
		return DeltaTime(_time_seconds - other._time_seconds);
	}

private:
	double _time_seconds;
};

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
    virtual void OnUpdate(DeltaTime delta_time) = 0;
    virtual void OnShutdown() = 0;

	Time GetTime() const;

	void Run();

protected:
	uint32_t GetScreenWidth() { return _params.screen_width; }
	uint32_t GetScreenHeight() { return _params.screen_height; }
	ResourceManager* GetResourceManager() { return _resource_manager; }
	void Quit() { _running = false; }

private:
	void Initialize();
	void Shutdown();

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
};