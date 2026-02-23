#include "ETriggerLocation.h"
#include <Renderer/Renderer.h>
#include <Game/GameFeatures.h>
#include <Game/GameplayLogic.h>
#include <Events/ChangeLocationEvent.h>
#include <Gameplay/WorldState.h>
#include <Events/ForceDialogStartEvent.h>
#include <filesystem>

void World::ETriggerLocation::Render(float deltaTime) {
    if (GameFeatures::isDebug) {
        Renderer::DrawSprite(texture, positionX, positionY, width, height);
    }

    if (!isLocationAvailable) {
        Renderer::DrawSprite(voidTextureId, positionX, positionY, width, height, false, true);
    }
}
    
void World::ETriggerLocation::ChangeLocation() {
    EventBus::instance().EmitEvent<ChangeLocationEvent>(locationName, spawnPointMatch);
}

void World::ETriggerLocation::OnSpawn(float x, float y, float w, float h) {
    EMapObject::OnSpawn(x, y, w, h);
    physicsTriggerId = Physics::CreateStaticRectangle(x, y, GetWidth(), GetHeight(), true, 0x02);
    if (texture == make_nnTex("levelchange") && *texture != 0) {
        voidTextureId = Renderer::TextureId("void-trigger"_nnTex);
    }
    else {
        voidTextureId = texture;
    }
}

void World::ETriggerLocation::OnDestroy() {
    EMapObject::OnDestroy();
    Physics::RemoveObject(physicsTriggerId);
}

void World::ETriggerLocation::ProcessOverlap() {
    const auto& overlapInfos = Physics::GetOverlapInfos();
    static bool isStopDialogShown = false;
    for (const auto& overlapInfo : overlapInfos) {
        if (overlapInfo.objectId1 == physicsTriggerId || overlapInfo.objectId2 == physicsTriggerId) {
            if (spawnPointMatch == "village_right" && GameplayLogic::GetCurrentGameActId() == GameActIds::Tutorial) {
                if (!isStopDialogShown) {
                    EventBus::instance().EmitEvent<ForceDialogStartEvent>("Utility", "no-time-to-leave");
                    isStopDialogShown = true;
                    return;
                }
                else {
                    return;
                }
            }

            ChangeLocation();
            break;
        }
    }
}

void World::ETriggerLocation::CheckStateWithTease() {
    const auto& registeredLocations = WorldState::GetCurrentState().registeredLocations;
    const auto shortLocationName = std::filesystem::path(locationName).filename().stem().string();
    if (!registeredLocations.contains(shortLocationName)) {
        isLocationAvailable = false;
        return;
    }
    else if (!registeredLocations.at(shortLocationName)) {
        isLocationAvailable = false;
        return;
    }
    isLocationAvailable = true;
}

void World::ETriggerLocation::CheckStateWithoutTease() {
    const auto& registeredLocations = WorldState::GetCurrentState().registeredLocations;
    const auto shortLocationName = std::filesystem::path(locationName).filename().stem().string();
    if (registeredLocations.contains(shortLocationName) && !registeredLocations.at(shortLocationName)) {
        isLocationAvailable = false;
    }
    else {
        isLocationAvailable = true;
    }
}

bool World::ETriggerLocation::Update(float deltaTime) {
    if (!EMapObject::Update(deltaTime)) {
        return false;
    }
    if (!tease) {
        CheckStateWithoutTease();
    }
    else {
        CheckStateWithTease();
    }

    if (isLocationAvailable) {
        ProcessOverlap();
    }

    return true;
}