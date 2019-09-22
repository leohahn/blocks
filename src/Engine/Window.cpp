#include "Han/Window.hpp"
#include "Han/Logger.hpp"
#include <assert.h>
#include <glad/glad.h>
#include <SDL.h>

class SdlWindow : public Window
{
public:
    SdlWindow(const WindowOptions& opts)
        : _opts(opts)
    {
		ASSERT(_opts.title, "title not present");

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            LOG_ERROR("Failed to init SDL\n");
            exit(1);
        }
        
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);
        
        _raw_window =
            SDL_CreateWindow(_opts.title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _opts.width, _opts.height, SDL_WINDOW_OPENGL);
		SDL_SetWindowBordered(_raw_window, SDL_TRUE);

        _gl_context = SDL_GL_CreateContext(_raw_window);
		ASSERT(SDL_GL_SetSwapInterval(_opts.vsync) == 0, "failed to set vsync");
        SDL_GL_MakeCurrent(_raw_window, _gl_context);
        
        LOG_INFO("Initializing glad");
        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
            LOG_ERROR("Failed to initialize GLAD\n");
            exit(1);
        }

        glViewport(0, 0, _opts.width, _opts.height);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
    }

    ~SdlWindow()
    {
        SDL_GL_DeleteContext(_gl_context);
        SDL_DestroyWindow(_raw_window);
    }

	void SetEventCallback(EventCb cb) override
	{
		_event_cb = std::move(cb);
	}
    
    int32_t GetWidth() override
    {
        return _opts.width;
    }
    
    int32_t GetHeight() override
    {
        return _opts.height;
    }

	void UpdateInputs()
	{
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			SDL_Keycode ev_keycode = event.key.keysym.sym;
			int repeat_count = event.key.repeat;

			// TODO: Handle mobile events
			switch (event.type) {
				case SDL_APP_TERMINATING: {
					UNREACHABLE;
					break;
				}
				case SDL_QUIT: {
					QuitEvent ev;
					_event_cb(ev);
					break;
				}
				case SDL_KEYUP: {
					KeyReleaseEvent ev((KeyCode)ev_keycode);
					_event_cb(ev);
					break;
				}
				case SDL_KEYDOWN: {
					KeyPressEvent ev((KeyCode)ev_keycode, repeat_count);
					_event_cb(ev);
					break;
				}
				case SDL_MOUSEBUTTONUP: {
					MouseButtonReleaseEvent ev(event.button.button);
					_event_cb(ev);
					break;
				}
				case SDL_MOUSEBUTTONDOWN: {
					MouseButtonPressEvent ev(event.button.button, event.button.clicks);
					_event_cb(ev);
					break;
				}
				case SDL_MOUSEMOTION: {
					auto motion = event.motion;
					MouseMoveEvent ev(motion.x, motion.xrel, motion.y, motion.yrel);
					_event_cb(ev);
					break;
				}
				case SDL_MOUSEWHEEL: {
					int x, y;
					if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
						x = event.wheel.x * -1;
						y = event.wheel.y * -1;
					} else {
						x = event.wheel.x;
						y = event.wheel.y;
					}
					MouseWheelEvent ev(x, y);
					_event_cb(ev);
					break;
				}
			}
		}
	}
    
    void OnUpdate() override
    {
		UpdateInputs();
        SDL_GL_SwapWindow(_raw_window);
    }

    void* GetNativeHandle() override
    {
        return _raw_window;
    }

private:
	WindowOptions _opts;
    SDL_Window* _raw_window = nullptr;
    SDL_GLContext _gl_context;
	Window::EventCb _event_cb;
};

Window*
Window::Create(Allocator* allocator, WindowOptions opts)
{
    assert(allocator);
    return allocator->New<SdlWindow>(opts);
}
