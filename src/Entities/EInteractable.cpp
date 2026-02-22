#include "EInteractable.h"
#include <Logger/Logger.h>
#include <Events/InteractWithEntityEvent.h>
#include <Map/Map.h>
#include <format>

bool World::EInteractable::Update(float deltaTime) {
    if (!Entity::Update(deltaTime)) {
        return false;
    }

    const auto& overlapInfos = Physics::GetOverlapInfos();
    for (const auto& overlapInfo : overlapInfos) {
        if (overlapInfo.objectId1 == physicsTriggerId || overlapInfo.objectId2 == physicsTriggerId) {
            MapManager::MarkAsInteracted(GetTagName());
            break;
        }
    }

    return true;
}

void World::EInteractable::Interact() {
    Logger::Log(std::format("[EInteractable] Interact with interactable: {}", get_nnInteractId(GetInteractId())));
    EventBus::instance().EmitEvent<InteractWithEntityEvent>(get_nnInteractId(GetInteractId()));
}

void World::EInteractable::CreatePhysicsObjects() {
    physicsTriggerId = Physics::CreateStaticRectangle(GetPosition().x, GetPosition().y, GetWidth(), GetHeight(), true, 1 << 7);
}

void World::EInteractable::RemovePhysicsObjects() {
    Physics::RemoveObject(physicsTriggerId);
}

void World::EInteractable::OnSpawn(float x, float y, float w, float h) {
    Entity::OnSpawn(x, y, w, h);
    CreatePhysicsObjects();
}

void World::EInteractable::OnDestroy() {
    EMapObject::OnDestroy();
    RemovePhysicsObjects();
}
