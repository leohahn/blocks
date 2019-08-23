#pragma once

#include <SDL_event.h>

class PlayerInputContext
{
public:
    enum Direction
    {
        kDirectionLeft,
        kDirectionRight,
    };

    Direction GetDirection() const;

private:
    static void OnInputEvent(SDL_Event ev, void* user_data);
};
