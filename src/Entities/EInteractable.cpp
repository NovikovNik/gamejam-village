#include "EInteractable.h"
#include <Logger/Logger.h>
#include <Events/InteractWithEntityEvent.h>
#include <format>

bool World::EInteractable::Update(float deltaTime) {
    if (!Entity::Update(deltaTime)) {
        return false;
    }

    return true;
}

void World::EInteractable::Interact() {
    Logger::Log(std::format("[EInteractable] Interact with interactable: {}", get_nnInteractId(GetInteractId())));
    EventBus::instance().EmitEvent<InteractWithEntityEvent>(get_nnInteractId(GetInteractId()));
}