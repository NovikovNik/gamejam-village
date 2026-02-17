#include "EInteractable.h"
#include <Logger/Logger.h>
#include <format>

bool World::EInteractable::Update(float deltaTime) {
    if (!Entity::Update(deltaTime)) {
        return false;
    }

    return true;
}

void World::EInteractable::Interact() {
    Logger::Log(std::format("Interact with interactable: {}", get_nnInteractId(GetInteractId())));
}