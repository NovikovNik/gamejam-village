#pragma once

#include <glm/glm.hpp>
#include <SDL.h>

struct BoxColliderComponent {
    int width;
    int height;
    glm::vec2 offset;
    SDL_Color debugColor;

    BoxColliderComponent(int width = 0, int height = 0, glm::vec2 offset = glm::vec2(0), SDL_Color debugColor = {255, 0, 0, 255}) {
        this->width = width;
        this->height = height;
        this->offset = offset;
        this->debugColor = debugColor;
    }
};