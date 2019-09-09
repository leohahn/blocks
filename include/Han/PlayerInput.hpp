#pragma once

#include "InputSystem.hpp"
#include "Han/Allocator.hpp"
#include <SDL_events.h>

class PlayerInput
{
public:
    PlayerInput()
        : _moving_left(false)
        , _moving_right(false)
        , _moving_backwards(false)
        , _moving_forwards(false)
        , _turning_right(false)
        , _turning_left(false)
        , _turning_above(false)
        , _turning_below(false)
    {}

    void RegisterInputs(InputSystem* input_system, Allocator* allocator)
    {
        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonHold, SDLK_a, LeftInput, this, allocator);
        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonUp, SDLK_a, LeftRelease, this, allocator);

        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonHold, SDLK_d, RightInput, this, allocator);
        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonUp, SDLK_d, RightRelease, this, allocator);

        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonHold, SDLK_RIGHT, TurnRightInput, this, allocator);
        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonUp, SDLK_RIGHT, TurnRightRelease, this, allocator);

        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonHold, SDLK_LEFT, TurnLeftInput, this, allocator);
        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonUp, SDLK_LEFT, TurnLeftRelease, this, allocator);

        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonHold, SDLK_UP, TurnAboveInput, this, allocator);
        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonUp, SDLK_UP, TurnAboveRelease, this, allocator);

        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonHold, SDLK_DOWN, TurnBelowInput, this, allocator);
        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonUp, SDLK_DOWN, TurnBelowRelease, this, allocator);

        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonHold, SDLK_s, BackInput, this, allocator);
        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonUp, SDLK_s, BackRelease, this, allocator);

        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonHold, SDLK_w, ForwardInput, this, allocator);
        input_system->AddKeyboardEventListener(
            kKeyboardEventButtonUp, SDLK_w, ForwardRelease, this, allocator);
    }

    bool IsMovingLeft() const { return _moving_left; }
    bool IsMovingRight() const { return _moving_right; }
    bool IsMovingForwards() const { return _moving_forwards; }
    bool IsMovingBackwards() const { return _moving_backwards; }
    bool IsTurningLeft() const { return _turning_left; }
    bool IsTurningRight() const { return _turning_right; }
    bool IsTurningAbove() const { return _turning_above; }
    bool IsTurningBelow() const { return _turning_below; }

private:
    static void LeftInput(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_moving_left = true;
    }

    static void LeftRelease(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_moving_left = false;
    }

    static void RightInput(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_moving_right = true;
    }

    static void RightRelease(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_moving_right = false;
    }

    static void TurnRightInput(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_turning_right = true;
    }

    static void TurnRightRelease(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_turning_right = false;
    }

    static void TurnLeftInput(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_turning_left = true;
    }

    static void TurnLeftRelease(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_turning_left = false;
    }

    static void TurnAboveInput(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_turning_above = true;
    }

    static void TurnAboveRelease(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_turning_above = false;
    }

    static void TurnBelowInput(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_turning_below = true;
    }

    static void TurnBelowRelease(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_turning_below = false;
    }

    static void BackInput(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_moving_backwards = true;
    }

    static void BackRelease(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_moving_backwards = false;
    }

    static void ForwardInput(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_moving_forwards = true;
    }

    static void ForwardRelease(SDL_Event ev, void* user_data)
    {
        auto ps = (PlayerInput*)user_data;
        ps->_moving_forwards = false;
    }

    bool _moving_left : 1;
    bool _moving_right : 1;
    bool _moving_backwards : 1;
    bool _moving_forwards : 1;
    bool _turning_right : 1;
    bool _turning_left : 1;
    bool _turning_above : 1;
    bool _turning_below : 1;
};
