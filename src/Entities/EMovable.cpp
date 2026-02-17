#include "EMovable.h"

bool World::EMovable::Update(float deltaTime) {
    if (!Entity::Update(deltaTime)) {
        return false;
    }

    positionX += velocityX * deltaTime;
    positionY += velocityY * deltaTime;

    velocityX = 0;
    velocityY = 0;
    return true;
}

void World::EMovable::AddImpulse(float x, float y) {
    velocityX += x;
    velocityY += y;
}
