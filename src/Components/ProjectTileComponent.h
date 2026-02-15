#pragma once
#include <SDL.h>

struct ProjectTileComponent {
    bool isFriendly;
    int hitPercentDamage;
    int duration;
    int startTime;

    ProjectTileComponent(bool isFriendly = false,
        int hitPercentDamage = 10,
        int duration = 0) {
            this->isFriendly = isFriendly;
            this->hitPercentDamage = hitPercentDamage;
            this->duration = duration;
            this->startTime = SDL_GetTicks64();
        }
};
