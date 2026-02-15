#pragma once
#include "SDL_timer.h"
#include <glm/glm.hpp>

struct ProjectTileEmitterComponent {
  int speed;
  int repeatFreaquency;
  int projectileDuration;
  int hitPercentDamage;
  bool isFriendly;
  int lastEmissionTime;
  bool autoFire;

  ProjectTileEmitterComponent(
      int speed = 100,
      int repeatFreaquency = 0,
      int prijectileDuration = 10000,
      int hitPercentDamage = 10,
      bool isFriendly = false,
      bool autoFire = true
  ) {
      this->speed = speed;
      this->repeatFreaquency = repeatFreaquency;
      this->projectileDuration = prijectileDuration;
      this->hitPercentDamage = hitPercentDamage;
      this->isFriendly = isFriendly;
      this->lastEmissionTime = SDL_GetTicks64();
      this->autoFire = autoFire;
  }
};
