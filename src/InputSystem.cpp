#include "InputSystem.hpp"

#include "Logger.hpp"
#include "Utils.hpp"
#include <SDL.h>

InputSystem::InputSystem()
    : _allocator(nullptr)
    , _last_keyboard_state(nullptr)
    , _should_quit(false)
{}

void
InputSystem::Create(Allocator* allocator)
{
    _allocator = allocator;
    size_t size = sizeof(uint8_t) * SDL_NUM_SCANCODES;
    _last_keyboard_state = (uint8_t*)(_allocator->Allocate(size));
    assert(_last_keyboard_state);

    for (int i = 0; i < kKeyboardEventMax; ++i) {
        _keyboard_map[i].allocator = allocator;
    }

    LOG_INFO("Input sytem initialized with %s of memory in %s allocator",
             Utils::GetPrettySize(size),
             allocator->GetName());
}

void
InputSystem::Destroy()
{
    for (int i = 0; i < kKeyboardEventMax; ++i) {
        _keyboard_map[i].Destroy();
    }
}

KeyboardEvent*
InputSystem::GetKeyboardEvents(const uint8_t* keyboard_state, SDL_KeyboardEvent ev, int* num_events)
{
    assert(num_events);
    SDL_Scancode ev_scancode = ev.keysym.scancode;

    static KeyboardEvent events[2];

    *num_events = 1;

    if (keyboard_state[ev_scancode]) {
        if (_last_keyboard_state[ev_scancode]) {
            events[0] = kKeyboardEventButtonHold;
        } else {
            events[0] = kKeyboardEventButtonDown;
            events[1] = kKeyboardEventButtonHold;
            *num_events = 2;
        }
    } else {
        if (_last_keyboard_state[ev_scancode]) {
            events[0] = kKeyboardEventButtonUp;
        } else {
            assert(false && "This code path should not execute");
        }
    }

    return events;
}

bool
InputSystem::MatchesKeyboardEvent(SDL_Event event,
                                  KeyboardEvent keyboard_event,
                                  const uint8_t* keyboard_state)
{
    // NOTE: this functions assumes that event is a keyboard event, it will not check otherwise.
    SDL_Scancode ev_scancode = event.key.keysym.scancode;

    switch (keyboard_event) {
        case kKeyboardEventButtonUp: {
            return !keyboard_state[ev_scancode] && _last_keyboard_state[ev_scancode];
        }
        case kKeyboardEventButtonDown: {
            return keyboard_state[ev_scancode] && !_last_keyboard_state[ev_scancode];
        }
        case kKeyboardEventButtonHold: {
            return keyboard_state[ev_scancode];
        }
        default: {
            assert(false && "Invalid keyboard event");
            return false;
        }
    }
}

void
InputSystem::Update()
{
    const uint8_t* keyboard_state = SDL_GetKeyboardState(nullptr);

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        SDL_Keycode ev_keycode = event.key.keysym.sym;

        switch (event.type) {
            case SDL_APP_TERMINATING:
            case SDL_QUIT: {
                _should_quit = true;
                break;
            }
            case SDL_APP_LOWMEMORY: {
                LOG_WARN("Low memory in the system");
                break;
            }
            // See if the current event is a keyboard event.
            case SDL_KEYUP:
            case SDL_KEYDOWN: {
                for (int keyboard_event_it = 0; keyboard_event_it < kKeyboardEventMax;
                     ++keyboard_event_it) {
                    bool event_matches = MatchesKeyboardEvent(
                        event, (KeyboardEvent)keyboard_event_it, keyboard_state);

                    if (!event_matches) {
                        // The event does not match, ignore and continue for loop.
                        continue;
                    }

                    // TODO, PERFORMANCE: Consider using here a map instead of an array,
                    // this way we would not need to loop over each array seeing if the key matches
                    // or not.

                    Array<KeyboardEventListener>& listeners = _keyboard_map[keyboard_event_it];
                    for (size_t i = 0; i < listeners.len; ++i) {
                        bool key_matches = listeners[i].keycode == ev_keycode;
                        if (key_matches) {
                            assert(listeners[i].cb);
                            listeners[i].cb(event, listeners[i].user_data);
                        }
                    }
                }
                break;
            }
        }
    }

    // Copy current keyboard state to last keyboard state
    memcpy(_last_keyboard_state, keyboard_state, SDL_NUM_SCANCODES);
}

bool
InputSystem::ReceivedQuitEvent() const
{
    return _should_quit;
}

void
InputSystem::AddKeyboardEventListener(KeyboardEvent event,
                                      SDL_Keycode keycode,
                                      KeyboardEventCallback callback,
                                      void* user,
                                      Allocator* allocator)
{
    assert(event < kKeyboardEventMax);
    assert(callback);

    Array<KeyboardEventListener>* arr = &_keyboard_map[event];

    KeyboardEventListener listener = {};
    listener.cb = callback;
    listener.user_data = user;
    listener.keycode = keycode;

    arr->PushBack(listener);
}
