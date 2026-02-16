#include "EBox.h"
#include <Map/Map.h>
#include <Entities/EPit.h>

void World::EBox::OnSpawn() {
    const auto textureId = Renderer::TextureId("box"_nnTex);
    LoadData(textureId, 0, 0, 64, 64);
}

bool World::EBox::Update(float deltaTime) {
    if (!EMovable::Update(deltaTime)) {
        return false;
    }
    
    return true;
}

void World::EBox::Render(float deltaTime) {
    EMovable::Render(deltaTime);
}


void World::EBox::Move(float dirX, float dirY, float speed) {
    AddImpulse(dirX * speed, dirY * speed);

    std::vector<World::EPit*> pits;
    MapManager::GetEntitiesContainer().FindEntities(pits);
    for (const auto& pit : pits) {
        if (pit->IsBoxNameMatch(boxName)) {
            const auto pitPosition = pit->GetPosition();
            const auto boxPosition = GetPosition();
            const auto distance = glm::distance(pitPosition, boxPosition);
            if (distance < 8) {
                pit->Destroy();
                Destroy();
            }
            return;
        }
    }
}
