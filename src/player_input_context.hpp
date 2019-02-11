#ifndef BLOCKS_PLAYER_INPUT_CONTEXT_HPP
#define BLOCKS_PLAYER_INPUT_CONTEXT_HPP

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

#endif // BLOCKS_PLAYER_INPUT_CONTEXT_HPP
