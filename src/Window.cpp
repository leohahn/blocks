#include "Window.hpp"
#include "Logger.hpp"
#include <assert.h>
#include <glad/glad.h>
#include <SDL.h>

class SdlWindow : public Window
{
public:
    SdlWindow(const WindowOptions& opts)
        : _title(opts.title)
        , _width(opts.width)
        , _height(opts.height)
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            LOG_ERROR("Failed to init SDL\n");
            exit(1);
        }
        
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);
        
        _raw_window =
            SDL_CreateWindow(_title, 0, 0, _width, _height, SDL_WINDOW_OPENGL);
        _gl_context = SDL_GL_CreateContext(_raw_window);
        SDL_GL_MakeCurrent(_raw_window, _gl_context);
        
        LOG_INFO("Initializing glad");
        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        {
            LOG_ERROR("Failed to initialize GLAD\n");
            exit(1);
        }
        
        glViewport(0, 0, _width, _height);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
    }

    ~SdlWindow()
    {
        SDL_GL_DeleteContext(_gl_context);
        SDL_DestroyWindow(_raw_window);
    }
    
    virtual int32_t GetWidth() override
    {
        return _width;
    }
    
    virtual int32_t GetHeight() override
    {
        return _height;
    }
    
    virtual void SwapBuffers() override
    {
        SDL_GL_SwapWindow(_raw_window);
    }

private:
    const char* _title;
    int32_t _width;
    int32_t _height;
    SDL_Window* _raw_window = nullptr;
    SDL_GLContext _gl_context;
};

Window*
Window::Create(Allocator* allocator, WindowOptions opts)
{
    assert(allocator);
    return allocator->New<SdlWindow>(opts);
}
