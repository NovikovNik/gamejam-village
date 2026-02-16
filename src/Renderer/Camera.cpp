#include "Camera.h"
#include "../Entities/EMovable.h"

void World::Camera::Follow(const World::EMovable* target) {
    followTarget = target;
}

void World::Camera::Update(float dt) {
    if (!followTarget) return;

    positionX = followTarget->GetPosition().x;
    positionY = followTarget->GetPosition().y;
}

glm::vec2 World::Camera::GetPosition() const {
    return glm::vec2(positionX, positionY);
}
