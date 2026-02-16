#include "EColliders.h"

#include <Renderer/Renderer.h>

bool World::EColliders::Update(float deltaTime) {
    return Entity::Update(deltaTime);
}

void World::EColliders::Render(float deltaTime) {
    if (!bRender) {
        return;
    }
    const auto textureId = Renderer::TextureId("collider"_nnTex);
    for (const auto& collider : colliders) {
        Renderer::DrawSprite(textureId, collider.x, collider.y, collider.width, collider.height);
    }
}

void World::EColliders::LoadColliders(const std::vector<Collider>& colliders) {
    this->colliders = colliders;
}
