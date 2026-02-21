#include "Entity.h"

bool World::Entity::Update(float deltaTime) {
    return isValid;
}

void World::Entity::OnSpawn(float x, float y, float w, float h)
{
    positionX = x;
    positionY = y;

    if (width == 0) {
        width = w;
    }
    if (height == 0) {
        height = h;
    }
}
