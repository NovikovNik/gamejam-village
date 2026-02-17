#include "EMapObject.h"

bool World::EMapObject::Update(float deltaTime) {
    return Entity::Update(deltaTime);
}

void World::EMapObject::Render(float deltaTime) {
    Renderer::DrawSprite(texture, positionX, positionY, width, height);
}

void World::EMapObject::SetPosition(float x, float y) {
    positionX = x;
    positionY = y;
}

glm::vec2 World::EMapObject::GetPosition() const {
    return glm::vec2(positionX, positionY);
}

void World::EMapObject::LoadData(Renderer::TextureId texture, float x, float y, float width, float height) {
    this->texture = texture;
    this->positionX = x;
    this->positionY = y;
    this->width = width;
    this->height = height;
}
