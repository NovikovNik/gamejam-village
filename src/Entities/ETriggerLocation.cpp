#include "ETriggerLocation.h"
#include <Renderer/Renderer.h>
#include <Game/GameFeatures.h>
#include <Events/ChangeLocationEvent.h>

void World::ETriggerLocation::Render(float deltaTime) {
    if (GameFeatures::isDebug) {
        Renderer::DrawSprite(texture, positionX, positionY, width, height);
    }
}

void World::ETriggerLocation::ChangeLocation() {
    EventBus::instance().EmitEvent<ChangeLocationEvent>(locationName, spawnPointMatch);
}