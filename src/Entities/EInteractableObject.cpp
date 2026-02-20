#include "EInteractableObject.h"
#include <Logger/Logger.h>
#include <Events/InteractWithEntityEvent.h>
#include <format>

World::EInteractableObject::EInteractableObject(const std::string& name, float x, float y)
    : EInteractable(make_nnInteractId(name)) {
    const auto textureId = make_nnTex(std::format("object_{}", name));
    LoadData(textureId, x, y, 64, 64);
}

bool World::EInteractableObject::Update(float deltaTime) {
    return EInteractable::Update(deltaTime);
}

void World::EInteractableObject::Render(float deltaTime) {
    EInteractable::Render(deltaTime);
}

void World::EInteractableObject::Interact() {
    Logger::Log(std::format("[EInteractableObject] Interact: {}", get_nnInteractId(GetInteractId())));
    EventBus::instance().EmitEvent<InteractWithEntityEvent>(get_nnInteractId(GetInteractId()));
}
