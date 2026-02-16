#include "EBox.h"

void World::EBox::OnSpawn() {
    const auto textureId = Renderer::TextureId("box"_nnTex);
    LoadData(textureId, 0, 0, 32, 32);
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
