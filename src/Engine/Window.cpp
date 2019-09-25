#include "Han/Window.hpp"
#include "Han/Logger.hpp"
#include "Han/Collections/RobinHashMap.hpp"
#include <assert.h>
#include <glad/glad.h>
#include <SDL.h>

class SdlWindow : public Window
{
public:
    SdlWindow(const WindowOptions& opts)
        : _opts(opts)
		, _sdl_keys_to_han(256)
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

		BuildKeyMap();
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
					KeyCode* kc = _sdl_keys_to_han.Find(ev_keycode);
					int mod_flags = 0;
					if ((event.key.keysym.mod & KMOD_ALT) != 0) {
						mod_flags |= KeyMod_Alt;
					}
					if ((event.key.keysym.mod & KMOD_SHIFT) != 0) {
						mod_flags |= KeyMod_Shift;
					}
					if ((event.key.keysym.mod & KMOD_GUI) != 0) {
						mod_flags |= KeyMod_Super;
					}
					if ((event.key.keysym.mod & KMOD_CTRL) != 0) {
						mod_flags |= KeyMod_Ctrl;
					}
					KeyReleaseEvent ev(kc ? *kc : KeyCode_Unknown, mod_flags);
					_event_cb(ev);
					break;
				}
				case SDL_KEYDOWN: {
					KeyCode* kc = _sdl_keys_to_han.Find(ev_keycode);
					int mod_flags = 0;
					if ((event.key.keysym.mod & KMOD_ALT) != 0) {
						mod_flags |= KeyMod_Alt;
					}
					if ((event.key.keysym.mod & KMOD_SHIFT) != 0) {
						mod_flags |= KeyMod_Shift;
					}
					if ((event.key.keysym.mod & KMOD_GUI) != 0) {
						mod_flags |= KeyMod_Super;
					}
					if ((event.key.keysym.mod & KMOD_CTRL) != 0) {
						mod_flags |= KeyMod_Ctrl;
					}
					KeyPressEvent ev(kc ? *kc : KeyCode_Unknown, repeat_count, mod_flags);
					_event_cb(ev);
					break;
				}
				case SDL_MOUSEBUTTONUP: {
					switch (event.button.button) {
						case SDL_BUTTON_LEFT: {
							MouseButtonReleaseEvent ev(MouseButton_Left);
							_event_cb(ev);
							break;
						}
						case SDL_BUTTON_MIDDLE: {
							MouseButtonReleaseEvent ev(MouseButton_Middle);
							_event_cb(ev);
							break;
						}
						case SDL_BUTTON_RIGHT: {
							MouseButtonReleaseEvent ev(MouseButton_Right);
							_event_cb(ev);
							break;
						}
					}
					break;
				}
				case SDL_MOUSEBUTTONDOWN: {
					switch (event.button.button) {
						case SDL_BUTTON_LEFT: {
							MouseButtonPressEvent ev(MouseButton_Left, event.button.clicks);
							_event_cb(ev);
							break;
						}
						case SDL_BUTTON_MIDDLE: {
							MouseButtonPressEvent ev(MouseButton_Middle, event.button.clicks);
							_event_cb(ev);
							break;
						}
						case SDL_BUTTON_RIGHT: {
							MouseButtonPressEvent ev(MouseButton_Right, event.button.clicks);
							_event_cb(ev);
							break;
						}
					}
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
				case SDL_TEXTINPUT: {
					TextInputEvent ev(event.text.text);
					_event_cb(ev);
					break;
				}
				case SDL_WINDOWEVENT: {
					if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
						WindowResizeEvent ev(event.window.data1, event.window.data2);
						_event_cb(ev);
					}
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

	void BuildKeyMap()
	{
		_sdl_keys_to_han.Add(SDLK_SPACE, KeyCode_Space);
		_sdl_keys_to_han.Add(SDLK_QUOTE, KeyCode_Apostrophe);
		_sdl_keys_to_han.Add(SDLK_COMMA, KeyCode_Comma);
		_sdl_keys_to_han.Add(SDLK_MINUS, KeyCode_Minus);
		_sdl_keys_to_han.Add(SDLK_PERIOD, KeyCode_Period);
		_sdl_keys_to_han.Add(SDLK_SLASH, KeyCode_Slash);
		_sdl_keys_to_han.Add(SDLK_0, KeyCode_0);
		_sdl_keys_to_han.Add(SDLK_1, KeyCode_1);
		_sdl_keys_to_han.Add(SDLK_2, KeyCode_2);
		_sdl_keys_to_han.Add(SDLK_3, KeyCode_3);
		_sdl_keys_to_han.Add(SDLK_4, KeyCode_4);
		_sdl_keys_to_han.Add(SDLK_5, KeyCode_5);
		_sdl_keys_to_han.Add(SDLK_6, KeyCode_6);
		_sdl_keys_to_han.Add(SDLK_7, KeyCode_7);
		_sdl_keys_to_han.Add(SDLK_8, KeyCode_8);
		_sdl_keys_to_han.Add(SDLK_9, KeyCode_9);
		_sdl_keys_to_han.Add(SDLK_SEMICOLON, KeyCode_Semicolon);
		_sdl_keys_to_han.Add(SDLK_EQUALS, KeyCode_Equal);
		_sdl_keys_to_han.Add(SDLK_a, KeyCode_A);
		_sdl_keys_to_han.Add(SDLK_b, KeyCode_B);
		_sdl_keys_to_han.Add(SDLK_c, KeyCode_C);
		_sdl_keys_to_han.Add(SDLK_d, KeyCode_D);
		_sdl_keys_to_han.Add(SDLK_e, KeyCode_E);
		_sdl_keys_to_han.Add(SDLK_f, KeyCode_F);
		_sdl_keys_to_han.Add(SDLK_g, KeyCode_G);
		_sdl_keys_to_han.Add(SDLK_h, KeyCode_H);
		_sdl_keys_to_han.Add(SDLK_i, KeyCode_I);
		_sdl_keys_to_han.Add(SDLK_j, KeyCode_J);
		_sdl_keys_to_han.Add(SDLK_k, KeyCode_K);
		_sdl_keys_to_han.Add(SDLK_l, KeyCode_L);
		_sdl_keys_to_han.Add(SDLK_m, KeyCode_M);
		_sdl_keys_to_han.Add(SDLK_n, KeyCode_N);
		_sdl_keys_to_han.Add(SDLK_o, KeyCode_O);
		_sdl_keys_to_han.Add(SDLK_p, KeyCode_P);
		_sdl_keys_to_han.Add(SDLK_q, KeyCode_Q);
		_sdl_keys_to_han.Add(SDLK_r, KeyCode_R);
		_sdl_keys_to_han.Add(SDLK_s, KeyCode_S);
		_sdl_keys_to_han.Add(SDLK_t, KeyCode_T);
		_sdl_keys_to_han.Add(SDLK_u, KeyCode_U);
		_sdl_keys_to_han.Add(SDLK_v, KeyCode_V);
		_sdl_keys_to_han.Add(SDLK_w, KeyCode_W);
		_sdl_keys_to_han.Add(SDLK_x, KeyCode_X);
		_sdl_keys_to_han.Add(SDLK_y, KeyCode_Y);
		_sdl_keys_to_han.Add(SDLK_z, KeyCode_Z);
		_sdl_keys_to_han.Add(SDLK_LEFTBRACKET, KeyCode_LeftBracket);
		_sdl_keys_to_han.Add(SDLK_BACKSLASH, KeyCode_Backslash);
		_sdl_keys_to_han.Add(SDLK_RIGHTBRACKET, KeyCode_RightBracket);
		_sdl_keys_to_han.Add(SDLK_BACKQUOTE, KeyCode_GraveAccent);
		_sdl_keys_to_han.Add(SDLK_ESCAPE, KeyCode_Escape);
		_sdl_keys_to_han.Add(SDLK_RETURN, KeyCode_Enter);
		_sdl_keys_to_han.Add(SDLK_TAB, KeyCode_Tab);
		_sdl_keys_to_han.Add(SDLK_BACKSPACE, KeyCode_Backspace);
		_sdl_keys_to_han.Add(SDLK_INSERT, KeyCode_Insert);
		_sdl_keys_to_han.Add(SDLK_DELETE, KeyCode_Delete);
		_sdl_keys_to_han.Add(SDLK_RIGHT, KeyCode_Right);
		_sdl_keys_to_han.Add(SDLK_LEFT, KeyCode_Left);
		_sdl_keys_to_han.Add(SDLK_DOWN, KeyCode_Down);
		_sdl_keys_to_han.Add(SDLK_UP, KeyCode_Up);
		_sdl_keys_to_han.Add(SDLK_PAGEUP, KeyCode_PageUp);
		_sdl_keys_to_han.Add(SDLK_PAGEDOWN, KeyCode_PageDown);
		_sdl_keys_to_han.Add(SDLK_HOME, KeyCode_Home);
		_sdl_keys_to_han.Add(SDLK_END, KeyCode_End);
		_sdl_keys_to_han.Add(SDLK_CAPSLOCK, KeyCode_CapsLock);
		_sdl_keys_to_han.Add(SDLK_SCROLLLOCK, KeyCode_ScrollLock);
		_sdl_keys_to_han.Add(SDLK_NUMLOCKCLEAR, KeyCode_NumLock);
		_sdl_keys_to_han.Add(SDLK_PRINTSCREEN, KeyCode_PrintScreen);
		_sdl_keys_to_han.Add(SDLK_PAUSE, KeyCode_Pause);
		_sdl_keys_to_han.Add(SDLK_F1, KeyCode_F1);
		_sdl_keys_to_han.Add(SDLK_F2, KeyCode_F2);
		_sdl_keys_to_han.Add(SDLK_F3, KeyCode_F3);
		_sdl_keys_to_han.Add(SDLK_F4, KeyCode_F4);
		_sdl_keys_to_han.Add(SDLK_F5, KeyCode_F5);
		_sdl_keys_to_han.Add(SDLK_F6, KeyCode_F6);
		_sdl_keys_to_han.Add(SDLK_F7, KeyCode_F7);
		_sdl_keys_to_han.Add(SDLK_F8, KeyCode_F8);
		_sdl_keys_to_han.Add(SDLK_F9, KeyCode_F9);
		_sdl_keys_to_han.Add(SDLK_F10, KeyCode_F10);
		_sdl_keys_to_han.Add(SDLK_F11, KeyCode_F11);
		_sdl_keys_to_han.Add(SDLK_F12, KeyCode_F12);
		_sdl_keys_to_han.Add(SDLK_F13, KeyCode_F13);
		_sdl_keys_to_han.Add(SDLK_F14, KeyCode_F14);
		_sdl_keys_to_han.Add(SDLK_F15, KeyCode_F15);
		_sdl_keys_to_han.Add(SDLK_F16, KeyCode_F16);
		_sdl_keys_to_han.Add(SDLK_F17, KeyCode_F17);
		_sdl_keys_to_han.Add(SDLK_F18, KeyCode_F18);
		_sdl_keys_to_han.Add(SDLK_F19, KeyCode_F19);
		_sdl_keys_to_han.Add(SDLK_F20, KeyCode_F20);
		_sdl_keys_to_han.Add(SDLK_F21, KeyCode_F21);
		_sdl_keys_to_han.Add(SDLK_F22, KeyCode_F22);
		_sdl_keys_to_han.Add(SDLK_F23, KeyCode_F23);
		_sdl_keys_to_han.Add(SDLK_F24, KeyCode_F24);
		_sdl_keys_to_han.Add(SDLK_KP_0, KeyCode_Kp0);
		_sdl_keys_to_han.Add(SDLK_KP_1, KeyCode_Kp1);
		_sdl_keys_to_han.Add(SDLK_KP_2, KeyCode_Kp2);
		_sdl_keys_to_han.Add(SDLK_KP_3, KeyCode_Kp3);
		_sdl_keys_to_han.Add(SDLK_KP_4, KeyCode_Kp4);
		_sdl_keys_to_han.Add(SDLK_KP_5, KeyCode_Kp5);
		_sdl_keys_to_han.Add(SDLK_KP_6, KeyCode_Kp6);
		_sdl_keys_to_han.Add(SDLK_KP_7, KeyCode_Kp7);
		_sdl_keys_to_han.Add(SDLK_KP_8, KeyCode_Kp8);
		_sdl_keys_to_han.Add(SDLK_KP_9, KeyCode_Kp9);
		_sdl_keys_to_han.Add(SDLK_KP_DECIMAL, KeyCode_KpDecimal);
		_sdl_keys_to_han.Add(SDLK_KP_DIVIDE, KeyCode_KpDivide);
		_sdl_keys_to_han.Add(SDLK_KP_MULTIPLY, KeyCode_KpMultiply);
		_sdl_keys_to_han.Add(SDLK_KP_MINUS, KeyCode_KpSubtract);
		_sdl_keys_to_han.Add(SDLK_KP_PLUS, KeyCode_KpAdd);
		_sdl_keys_to_han.Add(SDLK_KP_ENTER, KeyCode_KpEnter);
		_sdl_keys_to_han.Add(SDLK_KP_EQUALS, KeyCode_KpEqual);
		_sdl_keys_to_han.Add(SDLK_LSHIFT, KeyCode_LeftShift);
		_sdl_keys_to_han.Add(SDLK_LCTRL, KeyCode_LeftControl);
		_sdl_keys_to_han.Add(SDLK_LALT, KeyCode_LeftAlt);
		_sdl_keys_to_han.Add(SDLK_LGUI, KeyCode_LeftSuper);
		_sdl_keys_to_han.Add(SDLK_RSHIFT, KeyCode_RightShift);
		_sdl_keys_to_han.Add(SDLK_RCTRL, KeyCode_RightControl);
		_sdl_keys_to_han.Add(SDLK_RALT, KeyCode_RightAlt);
		_sdl_keys_to_han.Add(SDLK_RGUI, KeyCode_RightSuper);
		_sdl_keys_to_han.Add(SDLK_MENU, KeyCode_Menu);
	}

private:
	WindowOptions _opts;
    SDL_Window* _raw_window = nullptr;
    SDL_GLContext _gl_context;
	Window::EventCb _event_cb;
	RobinHashMap<SDL_Keycode, KeyCode> _sdl_keys_to_han;
};

Window*
Window::Create(Allocator* allocator, WindowOptions opts)
{
    assert(allocator);
    return allocator->New<SdlWindow>(opts);
}
