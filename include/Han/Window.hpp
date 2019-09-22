#pragma once

#include <stdint.h>
#include <functional>
#include "Han/Allocator.hpp"
#include "Han/Events.hpp"

struct WindowOptions
{
    const char* title;
    int width;
    int height;
	bool vsync;
    
    WindowOptions()
        : title("window")
        , width(1024)
        , height(768)
		, vsync(true)
    {}
};

class Window
{
public:
	using EventCb = std::function<void(Event&)>;

    virtual ~Window() = default;
    virtual int32_t GetWidth() = 0;
    virtual int32_t GetHeight() = 0;
    virtual void OnUpdate() = 0;
    virtual void* GetNativeHandle() { return nullptr; }
	virtual void SetEventCallback(EventCb cb) = 0;
    
    static Window* Create(Allocator* allocator, WindowOptions opts);
};
