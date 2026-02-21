#include "ETriggerLocation.h"
#include <Renderer/Renderer.h>
#include <Game/GameFeatures.h>
#include <Events/ChangeLocationEvent.h>
#include <Gameplay/WorldState.h>
#include <filesystem>

void World::ETriggerLocation::Render(float deltaTime) {
    if (GameFeatures::isDebug) {
        Renderer::DrawSprite(texture, positionX, positionY, width, height);
    }

    if (!isLocationAvailable) {
        Renderer::DrawSprite(voidTextureId, positionX, positionY, width, height);
    }
}

void World::ETriggerLocation::ChangeLocation() {
    EventBus::instance().EmitEvent<ChangeLocationEvent>(locationName, spawnPointMatch);
}

void World::ETriggerLocation::OnSpawn(float x, float y, float w, float h) {
    EMapObject::OnSpawn(x, y, w, h);
    physicsTriggerId = Physics::CreateStaticRectangle(x, y, GetWidth(), GetHeight(), true, 0x02);
    voidTextureId = Renderer::TextureId("void-trigger"_nnTex);
}

void World::ETriggerLocation::OnDestroy() {
    EMapObject::OnDestroy();
    Physics::RemoveObject(physicsTriggerId);
}


bool World::ETriggerLocation::Update(float deltaTime) {
    if (!EMapObject::Update(deltaTime)) {
        return false;
    }

    const auto& registeredLocations = WorldState::GetCurrentState().registeredLocations;
    const auto shortLocationName = std::filesystem::path(locationName).filename().stem().string();
    if (registeredLocations.contains(shortLocationName) && !registeredLocations.at(shortLocationName)) {
        isLocationAvailable = false;
    }
    else {
        isLocationAvailable = true;
    }

    if (!isLocationAvailable) {
        return true;
    }

    const auto& overlapInfos = Physics::GetOverlapInfos();
    for (const auto& overlapInfo : overlapInfos) {
        if (overlapInfo.objectId1 == physicsTriggerId || overlapInfo.objectId2 == physicsTriggerId) {
            ChangeLocation();
            break;
        }
    }

    return true;
}