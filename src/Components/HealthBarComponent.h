#pragma once

#include "SDL_pixels.h"

struct HealthBarComponent {
    int width;
    int height;
    SDL_Color color;

    HealthBarComponent(
        int width = 32,
        int height = 6,
        SDL_Color color = {255, 255, 255}):
    width(width),
    height(height),
    color(color) {};
};
