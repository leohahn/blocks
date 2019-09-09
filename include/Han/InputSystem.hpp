#pragma once

#include "Han/Collections/Array.hpp"
#include "Han/LinearAllocator.hpp"
#include <math.h>
#include <stdlib.h>
#include <SDL.h>

enum KeyboardEvent
{
    kKeyboardEventButtonDown = 0,
    kKeyboardEventButtonUp,
    kKeyboardEventButtonHold,
    kKeyboardEventMax,
};

typedef void (*KeyboardEventCallback)(SDL_Event ev, void* user_data);

struct KeyboardEventListener
{
    KeyboardEventCallback cb;
    void* user_data;
    SDL_Keycode keycode;
};

class InputSystem
{
public:
    InputSystem();

    void Create(Allocator* allocator);
    void Destroy();

    bool ReceivedQuitEvent() const;
    void Update();
    void AddKeyboardEventListener(KeyboardEvent event,
                                  SDL_Keycode keycode,
                                  KeyboardEventCallback callback,
                                  void* user,
                                  Allocator* allocator);

    static InputSystem* Make(LinearAllocator allocator);

private:
    KeyboardEvent* GetKeyboardEvents(const uint8_t* keyboard_state,
                                     SDL_KeyboardEvent ev,
                                     int* num_events);
    bool MatchesKeyboardEvent(SDL_Event event,
                              KeyboardEvent keyboard_event,
                              const uint8_t* keyboard_state);

private:
    // TODO: add maps for each kind of input (keyboard, mouse, etc.)

    // TODO: in order to be more granular about the keyboard events,
    // we can use a hash table here with a struct as the key.
    Allocator* _allocator;
    Array<KeyboardEventListener> _keyboard_map[kKeyboardEventMax];
    uint8_t* _last_keyboard_state;
    bool _should_quit;
};
