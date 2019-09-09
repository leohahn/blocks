#pragma once

#include <stdint.h>
#include "Han/Allocator.hpp"

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
    virtual ~Window() = default;
    virtual int32_t GetWidth() = 0;
    virtual int32_t GetHeight() = 0;
    virtual void SwapBuffers() = 0;
    virtual void* GetNativeHandle() { return nullptr; }
    
    static Window* Create(Allocator* allocator, WindowOptions opts);
};
