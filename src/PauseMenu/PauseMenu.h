#pragma once

#include <SDL3/SDL_keycode.h>

union SDL_Event;

namespace PauseMenu {

    void Toggle();
    void Open();
    void Close();
    bool IsOpen();

    /// Call every frame from Game::Render() when the menu is open.
    void UpdateAndRender();

    /// Call from Game::ProcessInput() for every SDL_EVENT_KEY_DOWN while IsOpen().
    /// Returns true if the key was consumed.
    bool HandleKeyDown(SDL_Keycode key);

    /// Call from Game::ProcessInput() for every mouse event (motion, button down/up).
    /// Returns true if the event was consumed.
    bool HandleEvent(SDL_Event* event);
}
