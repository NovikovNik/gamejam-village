#include "Camera.h"
#include "../Entities/EMovable.h"
#include "../Entities/EntitiesContainer.h"
#include "../Map/Map.h"
#include "Logger/Logger.h"
#include <glm/glm.hpp>
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

void World::Camera::SetScaleFactor(float scaleFactor) {
    this->scaleFactor = scaleFactor;
}

void World::Camera::Update(float dt) {
    if (!followTarget || !followTarget->IsValid()) {
        Unfollow();
        return;
    }

    // glm::vec2 pos = followTarget->GetPosition();
    // glm::vec2 vel = followTarget->GetVelocity();
    // // Смещение
    // glm::vec2 offset = vel * lookAheadTime;
    // float len = glm::length(offset);
    // if (len > lookAheadMax && len > 0.0f) {
    //     offset *= (lookAheadMax / len);
    // }
    glm::vec2 target = followTarget->GetPosition();
    float t = 1.0f- std::exp(-smoothSpeed * dt);
    // float t = 1.0f;

    positionX += (target.x - positionX) * t;
    positionY += (target.y - positionY) * t;

    // Временная система ограничений камеры на уровень
    std::string currentLocationName = MapManager::GetCurrentMapName();
    if (!currentLocationName.empty()) {
        if (currentLocationName == "backroad") {
            if (positionY > -25.0) {
                positionY = -25.0;
            }
            if (positionY < -25.0) {
                positionY = -25.0;
            }
            if (positionX < -28.0) {
                positionX = -28.0;
            }
            if (positionX > 128.0) {
                positionX = 128.0;
            }
        }
        if (currentLocationName == "crossroads") {
            if (positionY < -470.0) {
                positionY = -470.0;
            }
            if (positionY > 468.0) {
                positionY = 468.0;
            }
            if (positionX < -568.0) {
                positionX = -568.0;
            }
            if (positionX > 571.0) {
                positionX = 571.0;
            }
        }
        if (currentLocationName == "world-void") {
            if (positionX < -151.0) {
                positionX = -151.0;
            }
            if (positionX > 150.0) {
                positionX = 150.0;
            }
        }
        if (currentLocationName == "assembly-hall" || currentLocationName == "elders-house" || currentLocationName == "old-house") { // Все остальные локации помещения
            positionX = -3.4;
            positionY = -1.8;
        }
    }
}