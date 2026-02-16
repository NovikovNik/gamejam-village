#include "EMovable.h"

bool World::EMovable::Update(float deltaTime) {
    if (!Entity::Update(deltaTime)) {
        return false;
    }

    positionX += velocityX * deltaTime;
    positionY += velocityY * deltaTime;

    velocityX = 0;
    velocityY = 0;
    return true;
}

void World::EMovable::Render(float deltaTime) {
    Renderer::DrawSprite(texture, positionX, positionY, width, height);
}

void World::EMovable::AddImpulse(float x, float y) {
    velocityX += x;
    velocityY += y;
}

void World::EMovable::SetPosition(float x, float y) {
    positionX = x;
    positionY = y;
}

glm::vec2 World::EMovable::GetPosition() const {
    return glm::vec2(positionX, positionY);
}

void World::EMovable::LoadData(Renderer::TextureId texture, float x, float y, float width, float height) {
    this->texture = texture;
    this->positionX = x;
    this->positionY = y;
    this->width = width;
    this->height = height;
}
