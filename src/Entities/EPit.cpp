#include "EPit.h"

void World::EPit::OnSpawn(float x, float y, float w, float h) {
    EMovable::OnSpawn(x, y, w, h);
    const auto textureId = Renderer::TextureId("pit"_nnTex);
    LoadData(textureId, 64, 64);
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

bool World::EPit::IsBoxNameMatch(BoxName boxName) const {
    return matchBoxName == boxName;
}
