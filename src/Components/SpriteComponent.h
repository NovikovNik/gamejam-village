#pragma once

#include "SDL_render.h"
#include <glm/glm.hpp>
#include <SDL.h>
#include <string>

enum Direction {
    UP = 0,
    RIGHT = 1,
    DOWN = 2,
    LEFT = 3
};

struct SpriteComponent {
    std::string assetId;
    int zindex;
    int width;
    int height;
    bool isFixed;
    SDL_Rect srcRect;
    SDL_RendererFlip flip;
    Direction direction;

    SpriteComponent(
        std::string assetId = "",
        int zindex = 0,
        int width = 0,
        int height = 0,
        bool isFixed = false,
        int srcRectX = 0,
        int srcRectY = 0) {
            this->assetId = assetId;
            this->zindex = zindex;
            this->width = width;
            this->height = height;
            this->isFixed = isFixed;
            this->srcRect = {
                srcRectX,
                srcRectY,
                width,
                height
            };
            this->flip = SDL_FLIP_NONE;
    }
};
