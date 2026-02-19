#include "Camera.h"
#include "../Entities/EMovable.h"
#include "../Entities/EntitiesContainer.h"
#include "../Map/Map.h"
#include "Logger/Logger.h"
#include <format>

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
    ResetPosition();
}

void World::Camera::ResetPosition() {
    positionX = 0.0f;
    positionY = 0.0f;
}

void World::Camera::SetPosition(float x, float y) {
    positionX = x;
    positionY = y;
}

glm::vec2 World::Camera::GetPosition() const {
    return glm::vec2(positionX, positionY);
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

    // Временная система ограничений камеры на уровень
    std::string currentLocationName = MapManager::GetCurrentMapName();
    if (!currentLocationName.empty()) {
        if (currentLocationName == "world-entry-2") {
            if (positionY > 14.0) {
                positionY = 14.0;
            }
            if (positionX < -150.0) {
                positionX = -150.0;
            }
            if (positionX > 255.0) {
                positionX = 255.0;
            }
        }
    }
}