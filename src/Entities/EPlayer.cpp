#include "EPlayer.h"

bool World::EPlayer::Update(float deltaTime) {
    if (!EMovable::Update(deltaTime)) {
        return false;
    }

    AddImpulse(10, 0);
    return true;
}

void World::EPlayer::Render(float deltaTime) {
    EMovable::Render(deltaTime);
}
