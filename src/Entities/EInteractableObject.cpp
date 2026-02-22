#include "EInteractableObject.h"
#include <Logger/Logger.h>
#include <Events/InteractWithEntityEvent.h>
#include <Map/Map.h>
#include <format>

World::EInteractableObject::EInteractableObject(const std::string& name)
    : EInteractable(make_nnInteractId(name)) {
    tagName = { name, "object" };
    texture = make_nnTex(std::format("object_{}", name));
}

bool World::EInteractableObject::Update(float deltaTime) {
    if (!EInteractable::Update(deltaTime)) {
        return false;
    }

    return true;
}

void World::EInteractableObject::Render(float deltaTime) {
    EInteractable::Render(deltaTime);
}

void World::EInteractableObject::Interact() {
    Logger::Log(std::format("[EInteractableObject] Interact: {}", get_nnInteractId(GetInteractId())));
    EventBus::instance().EmitEvent<InteractWithEntityEvent>(get_nnInteractId(GetInteractId()));
}
