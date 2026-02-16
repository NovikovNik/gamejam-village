#include "EPit.h"

void World::EPit::OnSpawn() {
    const auto textureId = Renderer::TextureId("pit"_nnTex);
    LoadData(textureId, 0, 0, 32, 32);
}

bool World::EPit::Update(float deltaTime) {
    if (!EMovable::Update(deltaTime)) {
        return false;
    }
    return true;
}

void World::EPit::Render(float deltaTime) {
    EMovable::Render(deltaTime);
}
