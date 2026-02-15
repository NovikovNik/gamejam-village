#pragma once

#include "SDL_pixels.h"
#include <glm/glm.hpp>
#include <string>

struct TextLabelComponent {
    glm::vec2 position;
    std::string text;
    std::string assetId;
    SDL_Color color;
    bool isFixed;

    TextLabelComponent(
        glm::vec2 pos = glm::vec2(0, 0),
        std::string text = "",
        std::string assetId = "",
        SDL_Color color = {0,0,0},
        bool isFixed = true):
            position(pos),
            text(text),
            assetId(assetId),
            color(color),
            isFixed(isFixed) {};
};
