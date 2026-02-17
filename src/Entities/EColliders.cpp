#include "EColliders.h"

#include <Renderer/Renderer.h>
#include "../Game/GameFeatures.h"

bool World::EColliders::Update(float deltaTime) {
    return Entity::Update(deltaTime);
}

void World::EColliders::Render(float deltaTime) {
    if (!bRender) {
        return;
    }
    if (GameFeatures::isDebug) { 
        // const auto textureId = Renderer::TextureId("collider"_nnTex);
        for (const auto& collider : colliders) {
            Renderer::DrawRectangle(collider.x, collider.y, collider.width, collider.height, 0.0);
            // Renderer::DrawSprite(textureId, collider.x, collider.y, collider.width, collider.height);
        }
    }
}

void World::EColliders::LoadColliders(const std::vector<Collider>& colliders) {
    this->colliders = colliders;
}
