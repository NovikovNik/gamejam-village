#include "EBox.h"
#include <Map/Map.h>
#include <Entities/EPit.h>

void World::EBox::OnSpawn() {
    const auto textureId = Renderer::TextureId("box"_nnTex);
    LoadData(textureId, 0, 0, 32, 32);
}

bool World::EBox::Update(float deltaTime) {
    if (!EMovable::Update(deltaTime)) {
        return false;
    }

    std::vector<World::EPit*> pits;
    MapManager::GetEntitiesContainer().FindEntities(pits);
    for (const auto& pit : pits) {
        if (pit->IsBoxNameMatch(boxName)) {
            return true;
        }
    }
    
    return true;
}

void World::EBox::Render(float deltaTime) {
    EMovable::Render(deltaTime);
}
