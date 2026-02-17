#include "GameStates.h"
#include <SDL3/SDL.h>

void GameStates::SyncMovementFromKeyboard(bool fourDirectionsOnly) {
    const bool* state = SDL_GetKeyboardState(nullptr);
    bool keyW = state[SDL_SCANCODE_W] || state[SDL_SCANCODE_UP];
    bool keyA = state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT];
    bool keyS = state[SDL_SCANCODE_S] || state[SDL_SCANCODE_DOWN];
    bool keyD = state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT];
    if (fourDirectionsOnly) {
        ResetMovement();
        if (keyW) w = true; else if (keyS) s = true; else if (keyA) a = true; else if (keyD) d = true;
    } else {
        w = keyW; a = keyA; s = keyS; d = keyD;
    }
}