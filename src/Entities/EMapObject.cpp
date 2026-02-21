#include "EMapObject.h"
#include <Game/GameFeatures.h>

bool World::EMapObject::Update(float deltaTime) {
    return Entity::Update(deltaTime);
}

void World::EMapObject::Render(float deltaTime) {
    if (texture != testTextureId) {
        // Хак, чтобы не рисовать для Interactible без текстуры (триггеры) тестовую текстуру
        Renderer::DrawSprite(texture, positionX, positionY, width, height);
    }
//    if (GameFeatures::isDebug) {
//        Renderer::DrawRectangle(GetPosition().x, GetPosition().y, GetWidth(), GetHeight(), 0.0);
//    }
}

void World::EMapObject::LoadData(Renderer::TextureId texture, float width, float height) {
    this->texture = texture;
    this->width = width;
    this->height = height;
}
