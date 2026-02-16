#include "Camera.h"
#include "../Entities/EMovable.h"
#include "../Entities/EntitiesContainer.h"
#include "Logger/Logger.h"

void World::Camera::Follow(const World::EMovable* target) {
    if (!target) {
        Logger::Warn("Trying to follow null Entity");
        return;
    }
    if (target->IsValid()) {
        followTarget = target;
        return;
    }
    Logger::Warn("Trying to follow not valid Entity");
}

void World::Camera::Unfollow() {
    followTarget = nullptr;
}

void World::Camera::Update(float dt) {
    if (!followTarget || !followTarget->IsValid()) {
        Unfollow();
        return;
    }

    glm::vec2 target = followTarget->GetPosition();
    float t = 1.0f - std::exp(-smoothSpeed * dt);
    positionX += (target.x - positionX) * t;
    positionY += (target.y - positionY) * t;
}

glm::vec2 World::Camera::GetPosition() const {
    return glm::vec2(positionX, positionY);
}
